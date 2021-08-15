#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json-util.h"
#include "logger.h"
#include "read-lines.h"
#include "util.h"
#include "xalloc.h"

static bool has_items(const struct menu_item *item)
{
	return item->items && item->n_items > 0;
}

static void destroy_item(struct menu_item *item)
{
	if (has_items(item)) {
		for (size_t i = 0; i < item->n_items; ++i) {
			destroy_item(&item->items[i]);
		}
	}
	free(item->items);
	free(item->title);
	free(item->tag);
}

static int construct(struct menu_item *item, struct menu_item *parent,
	struct json_node *json, struct logger *log)
{
	if (json->kind != JN_MAP) {
		logger_printf(log, LOGGER_ERROR,
			"Menu item is not a dictionary\n");
		goto error_kind;
	}
	union json_node_data *got;
	if ((got = json_map_get(json, "title", TAKE_NODE | JN_STRING))) {
		item->title = got->str;
	} else {
		logger_printf(log, LOGGER_ERROR,
			"Menu item has no or invalid \"title\" attribute\n");
		goto error_title;
	}
	item->tag = NULL;
	item->items = NULL;
	if ((got = json_map_get(json, "items", JN_LIST))) {
		item->kind = ITEM_LINKS;
		struct json_node_data_list *list = &got->list;
		item->items = xmalloc(list->n_vals * sizeof(*item->items));
		item->n_items = 0;
		for (size_t i = 0; i < list->n_vals; ++i) {
			if (construct(&item->items[i], item, &list->vals[i], log))
				goto error_item;
			++item->n_items;
		}
	} else if ((got = json_map_get(json, "text", TAKE_NODE | JN_STRING))) {
		item->kind = ITEM_TEXT;
		item->tag = got->str;
		subst_native_dir_sep(item->tag);
		item->n_items = 0;
	} else if ((got = json_map_get(json, "tag", TAKE_NODE | JN_STRING))) {
		item->kind = ITEM_TAG;
		item->tag = got->str;
	} else {
		item->kind = ITEM_INERT;
	}
	item->place = 0;
	item->frame = 0;
	item->parent = parent;
	return 0;

error_item:
	for (size_t i = 0; i < item->n_items; ++i) {
		destroy_item(&item->items[i]);
	}
	free(item->title);
error_title:
error_kind:
	return -1;
}

int menu_init(struct menu *menu, const char *root_dir, struct screen_area *area,
	struct logger *log)
{
	char *fname = mid_cat(root_dir, DIRSEP, "menu.json");
	FILE *file = fopen(fname, "r");
	if (!file) {
		logger_printf(log, LOGGER_ERROR,
			"Menu file \"%s\" could not be opened\n", fname);
		goto error_fopen;
	}
	struct json_node jtree;
	if (parse_json_tree("menu", file, log, &jtree)) goto error_json_parse;
	menu->root = xmalloc(sizeof(*menu->root));
	if (construct(menu->root, NULL, &jtree, log)) goto error_json_format;
	menu->area = area;
	menu->current = menu->root;
	menu->root_dir = root_dir;
	menu->msg = "";
	menu->lines = NULL;
	menu->n_lines = 0;
	menu->needs_redraw = true;
	free_json_tree(&jtree);
	free(fname);
	return 0;

error_json_format:
	free(menu->root);
	free_json_tree(&jtree);
error_json_parse:
error_fopen:
	free(fname);
	return -1;
}

struct menu_item *menu_get_current(struct menu *menu)
{
	return menu->current;
}

struct menu_item *menu_get_selected(struct menu *menu)
{
	if (!menu->current
	 || menu->current->kind != ITEM_LINKS
	 || (size_t)menu->current->place >= menu->current->n_items)
		return NULL;
	return &menu->current->items[menu->current->place];
}

static void text_n_items(struct menu *menu, struct menu_item *text)
{
	size_t lines = (size_t)menu->area->height - 3;
	if (menu->n_lines > lines) {
		text->n_items = menu->n_lines - lines;
	} else {
		text->n_items = 1;
	}
}

int menu_scroll(struct menu *menu, int amount)
{
	struct menu_item *current = menu->current;
	if (current->kind == ITEM_TEXT) text_n_items(menu, current);
	if (current->n_items <= 0) return 0;
	int last_place = current->place;
	current->place =
		CLAMP(current->place + amount, 0, (int)current->n_items - 1);
	if (current->kind == ITEM_LINKS) {
		int height = menu->area->height - 2;
		if ((int)current->n_items < height) {
			current->frame = 0;
		} else {
			current->frame = CLAMP(current->frame,
				current->place - height + 1, current->place);
		}
	}
	menu->needs_redraw = menu->needs_redraw || current->place != last_place;
	return current->place - last_place;
}

static void enter(struct menu *menu, struct menu_item *into)
{
	menu->needs_redraw = true;
	menu->current = into;
}

enum menu_action menu_enter(struct menu *menu)
{
	struct menu_item *current = menu->current;
	if (!has_items(current)) return ACTION_BLOCKED;
	struct menu_item *into = &current->items[current->place];
	switch (into->kind) {
		FILE *txtfile;
		char *fname;
	case ITEM_INERT:
		return ACTION_BLOCKED;
	case ITEM_LINKS:
		enter(menu, into);
		return ACTION_WENT;
	case ITEM_TEXT:
		fname = mid_cat(menu->root_dir, DIRSEP, into->tag);
		if (!(txtfile = fopen(fname, "r"))
		 || !(menu->lines = read_lines(txtfile, &menu->n_lines))) {
			free(fname);
			menu_set_message(menu, "Failed to read file");
			return ACTION_BLOCKED;
		}
		free(fname);
		text_n_items(menu, into);
		enter(menu, into);
		return ACTION_WENT;
	case ITEM_TAG:
		menu->needs_redraw = true;
		return ACTION_TAG;
	}
	return ACTION_BLOCKED;
}

bool menu_escape(struct menu *menu)
{
	if (menu->lines) {
		for (size_t i = 0; i < menu->n_lines; ++i) {
			free(menu->lines[i].text);
		}
		free(menu->lines);
		menu->lines = NULL;
	}
	if (!menu->current->parent) return false;
	menu->needs_redraw = true;
	menu->current = menu->current->parent;
	return true;
}

void menu_mark_area_changed(struct menu *menu)
{
	menu->needs_redraw = true;
}

static void move_clear_line(struct menu *menu, int y, int x)
{
	move(y, 0);
	printw("%*s", menu->area->width, "");
	move(y, x);
}

void menu_draw(struct menu *menu)
{
	int i = 0;
	struct menu_item *current = menu->current;
	if (!menu->needs_redraw) return;
	menu->needs_redraw = false;
	move_clear_line(menu, 0, 0);
	attron(A_UNDERLINE);
	addstr(current->title);
	attroff(A_UNDERLINE);
	int height = menu->area->height - 1;
	if (current->kind == ITEM_LINKS) {
		for (int l = current->frame;
		     l < (int)current->n_items && ++i < height; ++l) {
			int reverse = l == current->place;
			move_clear_line(menu, i, 0);
			if (reverse) attron(A_REVERSE);
			printw("%2d. %s ", l + 1, current->items[l].title);
			if (reverse) attroff(A_REVERSE);
		}
	} else if (current->kind == ITEM_TEXT) {
		for (int l = current->place;
		     l < (int)menu->n_lines && ++i < height; ++l) {
			struct string *line = &menu->lines[l];
			mvprintw(i, 0, "%-*.*s",
				menu->area->width, (int)line->len, line->text);
		}
	}
	if (++i >= height) i = height;
	for (int j = i; j <= height; ++j) {
		move_clear_line(menu, j, 0);
	}
	mvaddstr(i, 0, menu->msg);
}

void menu_set_message(struct menu *menu, const char *msg)
{
	menu->needs_redraw = true;
	menu->msg = msg;
}

void menu_clear_message(struct menu *menu)
{
	menu->needs_redraw = true;
	menu->msg = "";
}

void menu_destroy(struct menu *menu)
{
	destroy_item(menu->root);
	free(menu->root);
	if (menu->lines) {
		for (size_t i = 0; i < menu->n_lines; ++i) {
			free(menu->lines[i].text);
		}
		free(menu->lines);
	}
}
