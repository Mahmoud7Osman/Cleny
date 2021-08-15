#include "json-util.h"
#include "grow.h"
#include "logger.h"
#include "table.h"
#include "util.h"
#include "xalloc.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

struct json_reader_ctx {
	char buf[BUFSIZ];
	const char *name;
	FILE *file;
	int line;
	struct logger *log;
};

static int refill(char **buf, size_t *size, void *ctx_v)
{
	struct json_reader_ctx *ctx = ctx_v;
	size_t got = fread(*buf, 1, *size, ctx->file);
	char *left = *buf;
	while ((left = memchr(left, '\n', got - (left - *buf)))) {
		++ctx->line;
		++left;
	}
	if (got < *size) {
		*size = got;
		return feof(ctx->file) ? 0 : -JSON_ERROR_ERRNO;
	}
	return 1;

}

static void print_json_error(const json_reader *rdr,
	const struct json_item *item)
{
	const char *msg;
	struct json_reader_ctx *ctx = *json_get_ctx((json_reader *)rdr);
	int line = ctx->line;
	size_t bufsiz;
	char *buf;
	json_get_buf(rdr, &buf, &bufsiz);
	if (item->val.erridx < bufsiz) {
		char *left = buf + item->val.erridx;
		while ((left = memchr(left, '\n', bufsiz - (left - buf)))) {
			--line;
			++left;
		}
	}
	switch (item->type) {
	case JSON_ERROR_MEMORY:
		msg = "Out of memory";
		break;
	case JSON_ERROR_NUMBER_FORMAT:
		msg = "Invalid number format";
		break;
	case JSON_ERROR_TOKEN:
		msg = "Invalid token";
		break;
	case JSON_ERROR_EXPECTED_STRING:
		msg = "Expected string literal";
		break;
	case JSON_ERROR_EXPECTED_COLON:
		msg = "Expected colon";
		break;
	case JSON_ERROR_BRACKETS:
		msg = "Mismatched brackets";
		break;
	case JSON_ERROR_UNCLOSED_QUOTE:
		msg = "Unclosed quote";
		break;
	case JSON_ERROR_ESCAPE:
		msg = "Invalid escape sequence";
		break;
	case JSON_ERROR_CONTROL_CHAR:
		msg = "Raw control character present in string";
		break;
	case JSON_ERROR_EXPECTED_VALUE:
		msg = "Expected value";
		break;
	case JSON_ERROR_IO:
		msg = "Input reading failed";
		break;
	case JSON_ERROR_ERRNO:
		msg = strerror(errno);
		break;
	default:
		msg = "Unknown error";
		break;
	}
	logger_printf(ctx->log, LOGGER_ERROR,
		"JSON parse error in '%s', line %d: %s\n",
		ctx->name, line, msg);
}

union json_node_data *json_map_get(struct json_node *map, const char *key,
	int kind)
{
	if (map->kind != JN_MAP) return NULL;
	void **got = table_get(&map->d.map, key);
	if (!got) return NULL;
	struct json_node *nd = *got;
	if (nd->taken) return NULL;
	if (nd->kind != (enum json_node_kind)(kind & ~TAKE_NODE)) return NULL;
	nd->taken = (kind & TAKE_NODE) != 0;
	return &nd->d;
}

static void free_json_pair(const char *key, struct json_node *val)
{
	free((char *)key);
	free_json_tree(val);
	free(val);
}

static void parse_node(json_reader *rdr, struct json_node *nd, char **keyp)
{
	size_t cap;
	struct json_item item;
	nd->taken = false;
	if (json_read_item(rdr, &item) < 0) {
		print_json_error(rdr, &item);
		nd->kind = JN_ERROR;
		return;
	}
	*keyp = item.key.bytes;
	switch (item.type) {
	case JSON_EMPTY:
		nd->kind = JN_EMPTY;
		break;
	case JSON_NULL:
		nd->kind = JN_NULL;
		break;
	case JSON_MAP:
		nd->kind = JN_MAP;
		table_init(&nd->d.map, 8);
		for (;;) {
			char *key = NULL;
			struct json_node *entry = xmalloc(sizeof(*entry));
			parse_node(rdr, entry, &key);
			if (entry->kind == JN_END_) {
				free_json_pair(key, entry);
				break;
			} else if (entry->kind == JN_ERROR) {
				free_json_pair(key, entry);
				const char *k;
				void **v;
				TABLE_FOR_EACH(&nd->d.map, k, v) {
					free_json_pair(k, *v);
				}
				table_free(&nd->d.map);
				nd->kind = JN_ERROR;
				return;
			}
			if (!key || table_add(&nd->d.map, key, entry))
				// With duplicate keys, the first is kept:
				free_json_pair(key, entry);
		}
		table_freeze(&nd->d.map);
		break;
	case JSON_LIST:
		nd->kind = JN_LIST;
		cap = 8;
		nd->d.list.n_vals = 0;
		nd->d.list.vals = xmalloc(cap * sizeof(*nd->d.list.vals));
		for (;;) {
			char *key = NULL;
			struct json_node *entry = GROWE(nd->d.list.vals,
				nd->d.list.n_vals, cap);
			parse_node(rdr, entry, &key);
			if (entry->kind == JN_END_) {
				--nd->d.list.n_vals;
				break;
			} else if (entry->kind == JN_ERROR) {
				free(key);
				for (size_t i = 0; i < nd->d.list.n_vals; ++i) {
					free_json_tree(&nd->d.list.vals[i]);
				}
				free(nd->d.list.vals);
				nd->kind = JN_ERROR;
				return;
			}
		}
		break;
	case JSON_STRING:
		nd->kind = JN_STRING;
		nd->d.str = item.val.str.bytes;
		break;
	case JSON_NUMBER:
		nd->kind = JN_NUMBER;
		nd->d.num = item.val.num;
		break;
	case JSON_BOOLEAN:
		nd->kind = JN_BOOLEAN;
		nd->d.boolean = item.val.boolean;
		break;
	case JSON_END_MAP:
	case JSON_END_LIST:
		nd->kind = JN_END_;
		break;
	default:
		break;
	}
}

int parse_json_tree(const char *name, FILE *file, struct logger *log,
	struct json_node *root)
{
	json_reader rdr;
	struct json_reader_ctx ctx;
	ctx.file = file;
	ctx.log = log;
	json_alloc(&rdr, NULL, 8, xmalloc, free, xrealloc);
	json_source(&rdr, ctx.buf, sizeof(ctx.buf), &ctx, refill);
	ctx.name = name;
	ctx.line = 1;
	char *key;
	parse_node(&rdr, root, &key);
	json_free(&rdr);
	fclose(file);
	return root->kind == JN_ERROR ? -1 : 0;
}

void free_json_tree(struct json_node *nd)
{
	if (nd->taken) return;
	switch (nd->kind) {
		const char *k;
		void **v;
	case JN_MAP:
		TABLE_FOR_EACH(&nd->d.map, k, v) {
			free_json_pair(k, *v);
		}
		table_free(&nd->d.map);
		break;
	case JN_LIST:
		for (size_t i = 0; i < nd->d.list.n_vals; ++i) {
			free_json_tree(&nd->d.list.vals[i]);
		}
		free(nd->d.list.vals);
		break;
	case JN_STRING:
		free(nd->d.str);
		break;
	default:
		break;
	}
}

int parse_json_vec(d3d_vec_s *vec, const struct json_node_data_list *list)
{
	int retval = 0;
	vec->x = vec->y = 0;
	if (list->n_vals < 1) return -1;
	if (list->vals[0].kind == JN_NUMBER) {
		vec->x = list->vals[0].d.num;
	} else {
		retval |= -1;
	}
	if (list->n_vals >= 2 && list->vals[1].kind == JN_NUMBER) {
		vec->y = list->vals[1].d.num;
	} else {
		retval |= -1;
	}
	return retval;
}

void escape_text_json(const char *text, struct string *buf, size_t *cap)
{
	while (*text) {
		switch (*text) {
		case '"':
			string_pushn(buf, cap, "\\\"", 2);
			break;
		case '\\':
			string_pushn(buf, cap, "\\\\", 2);
			break;
		case '\b':
			string_pushn(buf, cap, "\\b", 2);
			break;
		case '\f':
			string_pushn(buf, cap, "\\f", 2);
			break;
		case '\n':
			string_pushn(buf, cap, "\\n", 2);
			break;
		case '\r':
			string_pushn(buf, cap, "\\r", 2);
			break;
		case '\t':
			string_pushn(buf, cap, "\\t", 2);
			break;
		default:
			if ((unsigned char)*text < 32) {
				char *space = string_grow(buf, cap, 7);
				sprintf(space, "\\u%04X", (unsigned char)*text);
				buf->len -= 1; // Ignore NUL that was appended.
			} else {
				string_pushc(buf, cap, *text);
			}
			break;
		}
		++text;
	}
}

int scan_json_key(json_reader *rdr, const char *key, struct json_item *item)
{
	long depth = 0;
	do {
		if (json_read_item(rdr, item)) return -1;
		if (depth == 1
		 && item->key.bytes
		 && !strcmp(key, item->key.bytes))
			return 0;
		switch (item->type) {
		case JSON_LIST:
		case JSON_MAP:
			++depth;
			break;
		case JSON_END_LIST:
		case JSON_END_MAP:
			--depth;
			break;
		case JSON_EMPTY:
			depth = 0;
			break;
		case JSON_STRING:
			free(item->val.str.bytes);
			break;
		default:
			break;
		}
		free(item->key.bytes);
	} while (depth > 0);
	item->type = JSON_EMPTY;
	item->key.len = 0;
	item->key.bytes = NULL;
	return 0;
}

#if CTF_TESTS_ENABLED

#	include "libctf.h"
#	include "test-file.h"
#	include <assert.h>

#	define SOURCE(text) \
	struct json_node root; \
	struct logger logger; \
	logger_init(&logger); \
	do { \
		char str[] = text; \
		FILE *source = test_input(str, sizeof(str)); \
		parse_json_tree("(memory)", source, &logger, &root); \
	} while (0)

CTF_TEST(printed_json_error,
	SOURCE("{\"a\":1,\"b\":2");
	assert(root.kind == JN_ERROR);
	free_json_tree(&root);
)

CTF_TEST(json_vec,
	d3d_vec_s vec;
	SOURCE("[1,2]");
	assert(root.kind == JN_LIST);
	assert(!parse_json_vec(&vec, &root.d.list));
	assert(vec.x == 1);
	assert(vec.y == 2);
	free_json_tree(&root);
)

CTF_TEST(json_tree_map,
	SOURCE("{\"a\":1,\"b\":2}");
	assert(json_map_get(&root, "a", JN_NUMBER)->num == 1);
	assert(json_map_get(&root, "b", JN_NUMBER)->num == 2);
	free_json_tree(&root);
)

CTF_TEST(json_tree_list,
	SOURCE("[1,2]");
	assert(root.kind == JN_LIST);
	assert(root.d.list.n_vals == 2);
	assert(root.d.list.vals[0].d.num == 1);
	assert(root.d.list.vals[1].d.num == 2);
	free_json_tree(&root);
)

CTF_TEST(json_tree_nested,
	SOURCE("{\"a\":1,\"b\":[2,3]}");
	assert(json_map_get(&root, "a", JN_NUMBER)->num == 1);
	union json_node_data *list = json_map_get(&root, "b", JN_LIST);
	assert(list);
	assert(list->list.n_vals == 2);
	assert(list->list.vals[0].d.num == 2);
	assert(list->list.vals[1].d.num == 3);
	free_json_tree(&root);
)

CTF_TEST(json_node_taken,
	SOURCE("{\"a\":\"a\",\"b\":\"b\"}");
	union json_node_data *borrowed, *take_1, *take_2;
	borrowed = json_map_get(&root, "a", JN_STRING);
	take_1 = json_map_get(&root, "a", TAKE_NODE | JN_STRING);
	take_2 = json_map_get(&root, "a", TAKE_NODE | JN_STRING);
	assert(borrowed != NULL);
	assert(take_1 != NULL);
	assert(take_2 == NULL);
	free(take_1->str);
	free_json_tree(&root);
)

CTF_TEST(escapes_text_json,
	const char text[] = "ABC\n\"\x1BZ";
	const char expected[] = "ABC\\n\\\"\\u001BZ";
	struct string esc;
	size_t cap = 0;
	string_init(&esc, cap);
	escape_text_json(text, &esc, &cap);
	string_pushc(&esc, &cap, '\0');
	printf("escaped: \"%s\"\n", esc.text);
	assert(esc.len == sizeof(expected));
	assert(!strcmp(esc.text, expected));
)

CTF_TEST(scans_json_key,
	const char text[] = "{\"a\":{\"b\":2},\"b\":1}";
	json_reader rdr;
	json_alloc(&rdr, NULL, 2, xmalloc, free, xrealloc);
	json_source_string(&rdr, text, sizeof(text));
	struct json_item item;
	assert(!scan_json_key(&rdr, "b", &item));
	assert(item.val.num == 1);
)

CTF_TEST(scans_json_key_array,
	const char text[] = "[1,2]";
	json_reader rdr;
	json_alloc(&rdr, NULL, 2, xmalloc, free, xrealloc);
	json_source_string(&rdr, text, sizeof(text));
	struct json_item item;
	assert(!scan_json_key(&rdr, "b", &item));
	assert(item.type == JSON_EMPTY);
)

#endif /* CTF_TESTS_ENABLED */
