#ifndef JSON_UTIL_H_
#define JSON_UTIL_H_

#include "d3d.h"
#include "json.h"
#include "string.h"
#include "table.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

struct logger; // Weak dependency

// A node in a JSON tree
struct json_node {
	// The kind of the node. EMPTY is only possibly the top node, for an
	// empty file. ERROR is also only top level, indicating that the JSON
	// was invalid and an error was printed to stderr. Ignore END_. The
	// other kinds are clear.
	enum json_node_kind {
		JN_EMPTY,
		JN_NULL,
		JN_MAP,
		JN_LIST,
		JN_STRING,
		JN_NUMBER,
		JN_BOOLEAN,
		JN_ERROR,
		JN_END_
	} kind;
	// Whether the node is to be disposed of by other code.
	bool taken;
	// The type-specific node data.
	union json_node_data {
		// A JSON map from allocated NUL-terminated keys to node
		// pointers.
		table map;
		// A JSON list of n_vals sub-nodes.
		struct json_node_data_list {
			size_t n_vals;
			struct json_node *vals;
		} list;
		// An allocated NUL-terminated string.
		char *str;
		// A number.
		double num;
		// A boolean.
		bool boolean;
	} d;
};

// Try to get the data from a node in a given JSON map. NULL is returned if:
//  1. The map node is not actually a map.
//  2. The key is not present in the map.
//  3. The node found is not of the desired kind.
//  4. The node has already been taken (see below.)
// The last argument is a value of json_node_kind (the desired kind) possibly
// ORed with TAKE_NODE. TAKE_NODE specifies that the caller will take ownership
// of a node if it is found.
#define TAKE_NODE 0x1000
union json_node_data *json_map_get(struct json_node *map, const char *key,
	int kind);

// Parse a JSON tree from the file given into the root. Negative is returned if
// a system error occurred, but otherwise returned is zero. The name is used for
// errors. The file is closed.
int parse_json_tree(const char *name, FILE *file, struct logger *log,
	struct json_node *root);

// Completely free a JSON tree, including all the strings and stuff.
void free_json_tree(struct json_node *root);

// Parse a vector in the [x, y] form. Zero is returned unless the format is
// invalid, in which case -1 is returned and unparseable coordinates are zero.
int parse_json_vec(d3d_vec_s *vec, const struct json_node_data_list *list);

// Escape the text for use in JSON and append the result to the buffer. Quotes
// are not put around the escaped text. The input text is assumed to be UTF-8.
void escape_text_json(const char *text, struct string *buf, size_t *cap);

// Look for the key in the reader. Only depth 1 is searched. For example, if the
// desired key were "b" and the reader were at the value {"a":{"b":2},"b":1},
// the function would find 1, not 2. The result is placed in the item as with
// json_read_item. If none was found, an empty item is returned. If one was
// found, it is returned including an allocated key. If an error occurs, -1 is
// returned and the item is the error information. If no error occurred, 0 is
// returned.
int scan_json_key(json_reader *rdr, const char *key, struct json_item *item);

#endif /* JSON_UTIL_H_ */
