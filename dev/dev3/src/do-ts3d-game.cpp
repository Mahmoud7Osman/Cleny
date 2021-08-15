#include "do-ts3d-game.h"
#include "config.h"
#include "loader.h"
#include "logger.h"
#include "play-level.h"
#include "player.h"
#include "d3d.h"
#include "map.h"
#include "menu.h"
#include "save-state.h"
#include "ticker.h"
#include "ui-util.h"
#include "util.h"
#include "xalloc.h"
#include <curses.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

// Character cell width of menu. Must be enough to fit width 40 text.
#define MENU_WIDTH 41

// Screen state, including the menu and screensaver. The fields are public.
struct screen_state {
	// Whether the screen state is the right size for the physical screen.
	bool sized;
	struct menu_state {
		bool initialized;
		struct menu menu;
		struct screen_area area;
	} menu;
	struct title_state {
		bool initialized;
		d3d_camera *cam;
		// Parameter for camera direction and position:
		d3d_scalar t;
		d3d_board *board;
		struct screen_area area;
		struct color_map *color_map;
	} title;
};

// The initializer for the screen state excluding the menu/title stuff.
#define SCREEN_STATE_PARTIAL_INITIALIZER {  .sized = false, \
	.menu = { .initialized = false }, .title = { .initialized = false } }

// Load the menu part of the screen state. This is pretty much just menu_init
// except the area is provided internally. The return value is the same as
// menu_init's for success/failure.
static int load_menu_state(struct menu_state *state, const char *data_dir,
	struct logger *log)
{
	// Screen area not yet initialized:
	state->area = (struct screen_area) { 0, 0, 1, 1 };
	int ret = menu_init(&state->menu, data_dir, &state->area, log);
	state->initialized = !ret;
	return ret;
}

// Load the title screensaver map with the given loader, setting the camera and
// board in the title state. The area will be used to draw the scene later.
// -1 is returned for error.
static int load_title_state(struct title_state *state, struct loader *ldr)
{
	struct map *map = load_map(ldr, TITLE_SCREEN_MAP_NAME);
	if (!map) return -1;
	// Screen area not yet initialized:
	state->area = (struct screen_area) { 0, 0, 1, 1 };
	state->cam = camera_with_dims(state->area.width, state->area.height);
	state->t = 0.0;
	state->board = map->board;
	state->color_map = loader_color_map(ldr);
	state->initialized = true;
	return 0;
}

// Update the screen (the menu and the screensaver.)
static void do_screen_tick(struct screen_state *state)
{
	// Sync the screen size:
	if (!state->sized) {
		update_term_size();
		state->menu.area.width =
			COLS >= MENU_WIDTH ? MENU_WIDTH : COLS;
		state->menu.area.height = LINES;
		state->title.area.width = COLS - state->menu.area.width;
		state->title.area.height = LINES;
		state->title.area.x = state->menu.area.width;
		if (state->title.initialized) {
			d3d_free_camera(state->title.cam);
			state->title.cam = camera_with_dims(
				state->title.area.width,
				state->title.area.height);
		}
		if (state->menu.initialized)
			menu_mark_area_changed(&state->menu.menu);
		state->sized = true;
	}
	// Update the screen:
	if (state->title.initialized) {
		// theta is always increasing, but at a fluctuating rate so that
		// it pauses when the camera is facing the letters:
		d3d_scalar theta = -state->title.t
			- sin(state->title.t * 4.0 - PI) / 4.0;
		// r is at its highest when the camera is facing a letter:
		d3d_scalar r = (cos(theta * 4.0) + 0.5) / 1.5
			* TITLE_SCREEN_RADIUS;
		d3d_scalar x = r * cos(theta);
		d3d_scalar y = r * sin(theta);
		d3d_vec_s pos = {
			x + d3d_board_width(state->title.board) / 2.0,
			y + d3d_board_height(state->title.board) / 2.0
		};
		d3d_draw(state->title.cam, pos, theta,
			state->title.board, 0, NULL);
		display_frame(state->title.cam, &state->title.area,
			state->title.color_map);
		state->title.t += TITLE_SCREEN_SPEED;
	}
	if (state->menu.initialized) menu_draw(&state->menu.menu);
	refresh();
}

static void destroy_screen_state(struct screen_state *state)
{
	if (state->title.initialized) d3d_free_camera(state->title.cam);
	if (state->menu.initialized) menu_destroy(&state->menu.menu);
}

// Load the save state list, returning 0 for success and -1 for failure.
static int load_save_state(struct save_state *save,
	const char *state_file, struct logger *log)
{
	FILE *from = fopen(state_file, "r");
	// from closed by save_state_init:
	if (from && !save_state_init(save, from, log)) return 0;
	logger_printf(log, LOGGER_ERROR,
		"Unable to load save state from %s\n", state_file);
	return -1;
}

// Sync the save state with the file. The menu's message is set on error.
static int write_save_state(struct save_state *save, const char *state_file,
	struct logger *log, struct menu *menu)
{
	FILE *to = fopen(state_file, "w");
	int ret = 0;
	if (!to || save_state_write(save, to)) {
		int errnum = errno;
		FILE *print = logger_get_output(log, LOGGER_ERROR);
		if (print) {
			logger_printf(log, LOGGER_ERROR,
				"Failed to write state information to %s: %s\n",
				state_file, strerror(errnum));
			logger_printf(log, LOGGER_ERROR,
				"The information is below. Copy this to %s or"
				" to another location (see ts3d -h output):\n",
				state_file);
			save_state_write(save, print);
		}
		menu_set_message(menu, "SAVE ERROR!! QUIT NOW AND READ LOG!");
		beep();
		ret = -1;
	}
	if (to) fclose(to);
	return ret;
}

int do_ts3d_game(const char *data_dir, const char *state_file,
	struct logger *log)
{
	int ret = -1;
	struct loader ldr;
	loader_init(&ldr, data_dir);
	logger_free(loader_set_logger(&ldr, log));
	struct save_state save;
	if (load_save_state(&save, state_file, log)) goto error_save_state;
	// ncurses reads ESCDELAY and waits that many ms after an ESC key press.
	// This here is lowered from "1000":
	try_setenv("ESCDELAY", STRINGIFY(FRAME_DELAY), 0);
	initscr();
	cbreak();
	if (start_color() == ERR) {
		logger_printf(log, LOGGER_ERROR, "start_color() failed\n");
		goto error_color;
	}
	set_application_title("Thing Shooter 3D");
	timeout(0); // No delay for key presses.
	// The screen state including the menu and title screensaver:
	struct screen_state screen_state = SCREEN_STATE_PARTIAL_INITIALIZER;
	if (load_menu_state(&screen_state.menu, data_dir, log)) {
		logger_printf(log, LOGGER_ERROR, "Failed to load menu\n");
		goto error_menu;
	}
	if (load_title_state(&screen_state.title, &ldr)) {
		logger_printf(log, LOGGER_WARNING,
			"Failed to load title screensaver\n");
	} else {
		color_map_apply(loader_color_map(&ldr));
		loader_print_summary(&ldr);
	}
	ret = 0; // Things are successful by default when exiting after this.
	curs_set(0); // Hide the cursor.
	noecho(); // Hide input characters.
	keypad(stdscr, TRUE); // Automatically detect arrow keys and so on.
	struct ticker timer; // The ticker used throughout the game.
	ticker_init(&timer, FRAME_DELAY);
	int key; // Input key.
	for (;;) {
		// Alias for the menu, to make the code easier to read:
		struct menu *menu = &screen_state.menu.menu;
		// The selected menu item (used later):
		struct menu_item *selected;
		// The map prerequisite found (used later):
		char *prereq;
		tick(&timer);
		do_screen_tick(&screen_state);
		switch (key = getch()) {
		case 'd':
		case '\n':
		case KEY_ENTER:
		case KEY_RIGHT:
			menu_clear_message(menu);
			if (menu_enter(menu) == ACTION_TAG) {
				selected = menu_get_selected(menu);
				if (!selected || !selected->tag) break;
				if (!strcmp(selected->tag, "act/quit"))
					goto end;
				// By default, a tag loads a map by its name:
				prereq = map_prereq(&ldr, selected->tag);
				if (prereq
				 && !save_state_is_complete(&save, prereq)) {
					menu_set_message(menu, "Level locked");
					beep();
				} else if (play_level(data_dir, &save,
					selected->tag, &timer, log))
				{
					menu_set_message(menu,
						"Error loading map");
					beep();
				} else {
					menu_clear_message(menu);
					// Save the progress to the disk:
					ret = write_save_state(&save,
						state_file, log, menu);
					// Reset colors:
					color_map_apply(loader_color_map(&ldr));
					// Resize the screen state in case the
					// screen changed size during the level:
					screen_state.sized = false;
				}
				free(prereq);
			}
			break;
		case 'a':
		case ESC:
		case KEY_LEFT:
			// Leave submenu.
			if (menu_escape(menu)) menu_clear_message(menu);
			break;
		case 'w':
		case KEY_UP:
		case KEY_BACKSPACE:
		case KEY_SR:
			// Scroll up.
			menu_scroll(menu, -1);
			break;
		case 's':
		case ' ':
		case KEY_DOWN:
		case KEY_SF:
			// Scroll down.
			menu_scroll(menu, 1);
			break;
		case 'g':
			// Scroll to beginning.
			menu_scroll(menu, -999);
			break;
		case 'G':
			// Scroll to end.
			menu_scroll(menu, 999);
			break;
		case KEY_HOME:
			// Return to root menu.
			while (menu_escape(menu)) {
				menu_clear_message(menu);
			}
			break;
		case 'x':
			// Quit shortcut.
			goto end;
		case KEY_RESIZE:
			screen_state.sized = false;
			break;
		default:
			if (key >= '0' && key <= '9') {
				// Goto nth menu item.
				int to = key == '0' ? 9 : key - '0' - 1;
				menu_scroll(menu, -999);
				menu_scroll(menu, to);
			}
			break;
		}
	}
end:
error_menu:
	destroy_screen_state(&screen_state);
error_color:
	endwin();
	save_state_destroy(&save);
error_save_state:
	loader_free(&ldr);
	return ret;
}
