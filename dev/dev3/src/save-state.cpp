#include "save-state.h"
#include "json-util.h"
#include "string.h"
#include "util.h"
#include "xalloc.h"
#include <stdlib.h>

int save_state_init(struct save_state *save, FILE *from, struct logger *log)
{
	struct json_node jtree;
	if (parse_json_tree("save_state", from, log, &jtree)) return -1;
	table_init(&save->complete, 5); // This empty table will be populated.
	if (jtree.kind != JN_MAP) goto end;
	union json_node_data *got;
	if ((got = json_map_get(&jtree, "complete", JN_LIST))) {
		struct json_node_data_list *complete = &got->list;
		for (size_t i = 0; i < complete->n_vals; ++i) {
			struct json_node *node = &complete->vals[i];
			if (node->kind == JN_STRING)
				save_state_mark_complete(save, node->d.str);
		}
	} else if ((got = json_map_get(&jtree, "saves", JN_MAP))) {
		// With the old format, merge all the saves' completed levels
		// into one list.
		table *saves = &got->map;
		const char *UNUSED_VAR(key);
		void **val;
		TABLE_FOR_EACH(saves, key, val) {
			if ((got = json_map_get(*val, "complete", JN_LIST))) {
				struct json_node_data_list *complete =
					&got->list;
				for (size_t i = 0; i < complete->n_vals; ++i) {
					struct json_node *node =
						&complete->vals[i];
					if (node->kind == JN_STRING)
						save_state_mark_complete(save,
							node->d.str);
				}
			}
		}
	}
end:
	free_json_tree(&jtree);
	return 0;
}

bool save_state_is_complete(const struct save_state *save, const char *name)
{
	return table_get((table *)&save->complete, name) != NULL;
}

void save_state_mark_complete(struct save_state *save, const char *name)
{
	char *name_dup = str_dup(name);
	if (table_add(&save->complete, name_dup, NULL)) free(name_dup);
}

int save_state_write(struct save_state *save, FILE *to)
{
	struct string buf;
	size_t cap = 64;
	string_init(&buf, cap);
	string_pushz(&buf, &cap, "{\n \"complete\": ");
	if (table_count(&save->complete) > 0) {
		const char *before = "[";
		const char *key;
		void **UNUSED_VAR(val);
		TABLE_FOR_EACH(&save->complete, key, val) {
			string_pushz(&buf, &cap, before);
			string_pushz(&buf, &cap, "\n  \"");
			escape_text_json(key, &buf, &cap);
			before = "\",";
		}
		string_pushz(&buf, &cap, "\"\n ]\n}");
	} else {
		string_pushz(&buf, &cap, "[]\n}");
	}
	// Try to recover from errors where part of the data was written:
	size_t total_writ = 0;
	do {
		size_t writ = fwrite(buf.text + total_writ, 1,
			buf.len - total_writ, to);
		// Exit if no progress is being made:
		if (!writ) {
			free(buf.text);
			return -1;
		}
		clearerr(to);
		total_writ += writ;
	} while (total_writ < buf.len);
	free(buf.text);
	return 0;
}

void save_state_destroy(struct save_state *save)
{
	const char *key;
	void **UNUSED_VAR(val);
	TABLE_FOR_EACH(&save->complete, key, val) {
		free((char *)key);
	}
	table_free(&save->complete);
}

#if CTF_TESTS_ENABLED

#	include "libctf.h"
#	include "logger.h"
#	include "test-file.h"
#	include <assert.h>
#	include <string.h>

static const char new_save_text[] = "{\"complete\":[\"a\",\"b\",\"c\"]}";

static const char old_save_text[] =
	"{\"saves\":{"
		"\"PLAYER_1\":{\"complete\":[\"a\"]},"
		"\"PLAYER_2\":{\"complete\":[\"a\",\"b\"]},"
		"\"PLAYER_3\":{\"complete\":[\"c\"]}"
	"}}";

static void setup_save(struct save_state *save, const char *from_text)
{
	FILE *from = test_input(from_text, strlen(from_text));
	struct logger logger;
	logger_init(&logger);
	assert(!save_state_init(save, from, &logger));
}

char *save_to_string(struct save_state *save, size_t *n_writ)
{
	char *writ;
	int out_fd;
	FILE *to = test_output(&out_fd);
	save_state_write(save, to);
	fclose(to);
	test_read_output(out_fd, &writ, n_writ);
	writ = xrealloc(writ, *n_writ + 1);
	writ[*n_writ] = '\0';
	return writ;
}

CTF_TEST(write_save_valid,
	struct save_state save;
	setup_save(&save, new_save_text);
	size_t n_writ;
	char *writ = save_to_string(&save, &n_writ);
	struct logger logger;
	logger_init(&logger);
	struct json_node root;
	FILE *source = test_input(writ, n_writ);
	assert(parse_json_tree("(memory)", source, &logger, &root) == 0);
	logger_free(&logger);
	free_json_tree(&root);
	save_state_destroy(&save);
)

CTF_TEST(save_state_completed,
	struct save_state save;
	setup_save(&save, new_save_text);
	assert(!save_state_is_complete(&save, "LEVEL"));
	save_state_mark_complete(&save, "LEVEL");
	assert(save_state_is_complete(&save, "LEVEL"));
	assert(save_state_is_complete(&save, "a"));
	save_state_mark_complete(&save, "a");
	assert(save_state_is_complete(&save, "a"));
	size_t n_writ;
	char *writ = save_to_string(&save, &n_writ);
	assert(strstr(writ, "LEVEL"));
	free(writ);
	save_state_destroy(&save);
)

CTF_TEST(old_save_style,
	struct save_state save_new, save_old;
	setup_save(&save_new, new_save_text);
	setup_save(&save_old, old_save_text);
	size_t n_writ;
	char *str_new = save_to_string(&save_new, &n_writ);
	char *str_old = save_to_string(&save_old, &n_writ);
	assert(!strcmp(str_new, str_old));
	free(str_old);
	free(str_new);
	save_state_destroy(&save_old);
	save_state_destroy(&save_new);
)

CTF_TEST(empty_save_states,
	struct save_state save_none, save_obj, save_new, save_old;
	setup_save(&save_none, "");
	setup_save(&save_obj, "{}");
	setup_save(&save_new, "{\"complete\":[]}");
	setup_save(&save_old, "{\"saves\":{}}");
	size_t n_writ;
	char *str_none = save_to_string(&save_none, &n_writ);
	char *str_obj = save_to_string(&save_obj, &n_writ);
	char *str_old = save_to_string(&save_old, &n_writ);
	char *str_new = save_to_string(&save_new, &n_writ);
	assert(!strcmp(str_new, str_none));
	assert(!strcmp(str_new, str_obj));
	assert(!strcmp(str_new, str_old));
	free(str_new);
	free(str_old);
	free(str_obj);
	free(str_none);
	save_state_destroy(&save_new);
	save_state_destroy(&save_old);
	save_state_destroy(&save_obj);
	save_state_destroy(&save_none);
)

#endif /* CTF_TESTS_ENABLED */
