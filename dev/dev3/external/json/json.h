#ifndef JSON_H_
#define JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#ifdef JSON_WITH_STDIO
#include <stdio.h>
#endif

/* The persistent state between calls to json_read_item. Consider this an opaque
 * type. */
typedef struct {
	/* The memory allocation function, compatible with malloc. */
	void *(*alloc)(size_t);
	/* The memory deallocation function, compatible with free. */
	void  (*dealloc)(void *);
	/* The memory reallocation function, compatible with realloc. */
	void *(*resize)(void *, size_t);
	/* The buffer refilling function. See json_source at the bottom for
	 * details.*/
	int   (*refill)(char **buf, size_t *bufsiz, void *ctx);
	/* The data buffer given by refill. */
	char   *buf;
	/* The data buffer size given by refill. */
	size_t  bufsiz;
	/* The pointer passed to json_source. */
	void   *ctx;
	/* The index into the buffer of the next byte to be read. */
	size_t  head;
	/* The stack of containers. The small member is used if the SMALL_STACK
	 * flag is set. The stack should usually have fewer than 23 or so items,
	 * leading to the small stack to be used. The small stack is never used
	 * if a buffer is given with json_alloc. */
	union {
		/* The big, heap allocated stack. */
		struct json_big_stack {
			/* The allocated stack frames. */
			char  *frames;
			/* The number of stack frames used. */
			size_t size;
			/* The size of the allocation. */
			size_t cap;
		} big;
		/* The small, directly embedded stack. */
		struct json_small_stack {
			/* The embedded stack frames. */
			char          frames[sizeof(struct json_big_stack) - 1];
			/* The number of stack frames used. */
			unsigned char size;
			/* The capacity is always sizeof(frames). */
		} small;
	}       stack;
	/* Internal bitflags. See json.c for more details. */
	int     flags;
	/* Hack to make reading from an fd work. */
	int     fd;
} json_reader;

/* A parsed JSON string value encoded as UTF-8. Beware that JSON strings may
 * include NUL, so treating this as a C string is technically incorrect. It is
 * NUL terminated, though, for your convenience. This terminator is NOT included
 * in the length. */
struct json_string {
	/* The UTF-8 data for this string. This was allocated by the function
	 * given to the parser. */
	char  *bytes;
	/* The size of this string in bytes. */
	size_t len;
};

/* The type of a JSON item, which can be found in the type field of
 * struct json_item. */
enum json_type {
	/* There are currently no items on the stack. All opening brackete have
	 * been matched with closing ones. If this is returned twice in a row,
	 * then the input source is depleted. */
	JSON_EMPTY,
	/* The NULL value was parsed. */
	JSON_NULL,
	/* The beginning of a map has been parsed. Subsequent items will be
	 * coupled with keys. */
	JSON_MAP,
	/* The end of a map has been parsed. Subsequent items will not be
	 * coupled with keys. */
	JSON_END_MAP,
	/* The beginning of a list has been parsed. */
	JSON_LIST,
	/* The end of a list has been parsed. */
	JSON_END_LIST,
	/* A string has been parsed. the val.str field has been set. */
	JSON_STRING,
	/* A number has been parsed. the val.num field has been set. */
	JSON_NUMBER,
	/* A boolean has been parsed. the val.boolean field has been set to 1
	 * for true or 0 for false. */
	JSON_BOOLEAN,

	/* ERRORS are signified by a negative return value from json_read_item,
	 * and the val.erridx field is set to indicate where in the buffer the
	 * error occurred (this is not always relevant.) If an error occurs, the
	 * parser may not be used any longer. */

	/* One of the given memory allocation functions has failed. */
	JSON_ERROR_MEMORY,
	/* A number is in an invalid format. */
	JSON_ERROR_NUMBER_FORMAT,
	/* A single-token value (not a string) was not recognized. */
	JSON_ERROR_TOKEN,
	/* There was a trailing comma in a map, or a key did not start with '"'.
	 */
	JSON_ERROR_EXPECTED_STRING,
	/* A map ended after a key was parsed with no colon. */
	JSON_ERROR_EXPECTED_COLON,
	/* An opening bracket was incorrectly matched with a closing bracket. */
	JSON_ERROR_BRACKETS,
	/* A string had no closing '"' before the end of the input. */
	JSON_ERROR_UNCLOSED_QUOTE,
	/* An escape sequence was invalid. */
	JSON_ERROR_ESCAPE,
	/* An unescaped ASCII control character other than DELETE (0x7F) was
	 * present in a string. */
	JSON_ERROR_CONTROL_CHAR,
	/* There was a trailing comma in a list, or a colon was followed by the
	 * end of a map. */
	JSON_ERROR_EXPECTED_VALUE,

	/* USER ERRORS are never thrown by the library itself, but are reserved
	 * to be returned by the refill callback. */

	/* There was some error reading from a file. */
	JSON_ERROR_IO,
	/* An error occurred which is now stored in errno. */
	JSON_ERROR_ERRNO
};

/* The type-specific data in a json item. Many types have no associated field in
 * this union. */
union json_data {
	/* A parsed string (corresponding to JSON_STRING.) */
	struct json_string str;
	/* A parsed number (corresponding to JSON_NUMBER.) */
	double             num;
	/* A parsed boolean (corresponding to JSON_BOOLEAN.) A 1 is true, while
	 * a 0 is false. */
	int                boolean;
	/* The index into the data buffer where an error occurred (corresponding
	 * to any JSON_ERROR_* type.) THIS MAY BE OUTSIDE THE RANGE OF THE
	 * BUFFER, in which case the error is due to a premature end-of-file. */
	size_t             erridx;
};

/* An item in the stream of JSON. */
struct json_item {
	/* The key associated with this item. If a map is not currently being
	 * parsed, then key.bytes == NULL and key.len == 0. */
	struct json_string key;
	/* The type of this item. */
	enum json_type     type;
	/* The data specific to this item and its type. */
	union json_data    val;
};

/* FUNCTIONS
 * The followning functions manage the lifecycle of a json_reader. Unless
 * otherwise specified, no pointer parameter can be NULL.
 *
 * INITIALIZATION
 * Initialization is broken into two functions:
 *  1. json_alloc (example: json_alloc(&reader, 8, NULL, malloc, free, realloc))
 *  2. json_source
 *     (example: json_source(&reader, buf, sizeof(buf), file, my_refill_file))
 *   - json_source_string is a more specialized version
 *     (example: json_source_string(&reader, "[1,2,true]", 10))
 *   - json_source_file is for reading stdio files
 *     (example: json_source_file(&reader, buf, sizeof(buf), file))
 *  3. json_get_buf might be called at some point.
 * One function of each type should be called in that order.
 * See below for details.
 *
 * PARSING
 * The only function is json_read_item(&reader, &result), but this is the heart
 * of the library.
 *
 * DEALLOCATION
 * The only function is the simple json_free(&reader). */


/* Set the allocation routines for a parser.
 * PARAMETERS:
 *  1. reader: The parser to modify.
 *  2. stack: a memory buffer of size stacksiz, compatible with the dealloc and
 *            resize routines, or NULL if one should be generated using alloc.
 *  3. stacksiz: the initial size of the internal stack.
 *  4. alloc: The allocation function, compatible with malloc. Allocation always
 *            fails when this is NULL.
 *  5. dealloc: The deallocation function, compatible with free. When this is
 *              NULL, deallocation does nothing.
 *  6. resize: The reallocation function, compatible with realloc. Reallocation
 *             always fails when this is NULL.
 * RETURN VALUE: 0 on success, or -1 if the given allocation function failed to
 * allocate the stack of size stacksiz. */
int json_alloc(json_reader *reader,
	void *stack, size_t stacksiz,
	void *(*alloc)(size_t),
	void  (*dealloc)(void *),
	void *(*resize)(void *, size_t));

/* Set the input source for a parser.
 * PARAMETERS:
 *  1. reader: The parser to modify.
 *  2. buf: The initial data buffer (this can be NULL.) This will be refilled
 *          before it is ever read.
 *  3. bufsiz: The initial size of the data buffer.
 *  4. ctx: The context passed to the refilling function each call (this can be
 *          NULL.)
 *  5. refill: The function used to refill a data buffer. This is mainly geared
 *             reading files. If this is NULL, then input is effectively empty.
 *         PARAMETERS:
 *          1. buf: the pointer to the internal pointer to the buffer. This will
 *                  never be NULL, but if the pointed-to value is NULL, then a
 *                  new buffer should be allocated using the given allocation
 *                  routine beforehand, perhaps with the size pointed to in
 *                  bufsiz. The function is free to change the buffer pointer.
 *                  The buffer must be filled with UTF-8.
 *          2. bufsiz: The pointer to the size of the buffer. This is the size
 *                     of the buffer, and its value can be changed.
 *          3. ctx: The pointer which was passed to json_source.
 *         RETURN VALUE: A negative number less than 256 indicates an error,
 *         which will be reported when the current call to json_read_item
 *         returns. A positive number indicates that the source still has more
 *         to give beyond the data which was just read. Zero means that the
 *         source is depleted. */
void json_source(json_reader *reader,
	char *buf, size_t bufsiz, void *ctx,
	int (*refill)(char **buf, size_t *bufsiz, void *ctx));

/* Set the input source for a parser to the given buffer and no more.
 * PARAMETERS:
 *  1. reader: The parser.
 *  2. str: The text buffer.
 *  3. len: The length of the buffer. */
void json_source_string(json_reader *reader, const char *str, size_t len);

#ifdef JSON_WITH_STDIO
/* Read JSON from a FILE stream.
 * PARAMETERS:
 *  1. reader: The parser to modify.
 *  2. buf: The data buffer.
 *  3. bufsiz: The size of the data buffer.
 *  4. file: The stream from which to read. */
void json_source_file(json_reader *reader, char *buf, size_t bufsiz,
	FILE *file);
#endif /* JSON_WITH_STDIO */

#ifdef JSON_WITH_FD
/* Read JSON from a file descriptor. Seeking need not be supported. However,
 * this depends on a file consistently being able to fill a buffer of the same
 * size. It will not work on stdin, for example, which is line buffered.
 * PARAMETERS:
 *  1. reader: The parser to modify.
 *  2. buf: The data buffer.
 *  3. bufsiz: The size of the data buffer.
 *  4. file: The file descriptor from which to read. */
void json_source_fd(json_reader *reader, char *buf, size_t bufsiz, int fd);
#endif /* JSON_WITH_FD */

/* Read the next item from the input source.
 * PARAMETERS:
 *  1. reader: The parser being used.
 *  2. result: The place to put the result. See struct json_item,
 *             enum json_type, and union json_data.
 * RETURN VALUE: 0 for success, or a negative for failure. In the case of
 * failure, result->type will be set to some JSON_ERROR_* or whatever a failed
 * call the the given refill function returned. */
int json_read_item(json_reader *reader, struct json_item *result);

/* Get the parser's current buffer and its size. This is meant for pinpointing
 * on what character an error occurred.
 * PARAMETERS:
 *  1. reader: The parser.
 *  2. buf: The pointer which will be set to the buffer pointer.
 *  3. bufsiz: The pointer which will be set to the buffer size.
 */
void json_get_buf(const json_reader *reader, char **buf, size_t *bufsiz);

/* Get the number of characters in the buffer used already to parse JSON. If the
 * reader was initialized with json_source_string, returned is the number of
 * characters so far used in the given string.
 * PARAMETERS:
 *  1. reader: The parser.
 * RETURN VALUE: The number of characters used in the current buffer/string.
 */
size_t json_get_num_used(const json_reader *reader);

/* Get a pointer to the parser's context. If the context was set using
 * json_source, the return value is what the context was last set to and it can
 * be modified. If any other json_source_* function was used instead, the
 * contents are of unspecified value and cannot be modified. Avoid this function
 * unless you used plain old json_source. */
void **json_get_ctx(json_reader *reader);

/* Check the last reading error. If none occurred, 0 is returned and the given
 * pointers contain no meaningful information. If there has been an error, the
 * code and index** are put into the appropriate pointers. If the second or
 * third argument is NULL, that pointer will never be dereferenced and therefore
 * will never be set.
 *
 * ** See the erridx member of union json_data for details. */
int json_get_last_error(const json_reader *reader,
	enum json_type *code,
	size_t *erridx);

/* Deallocate all memory associated with the given parser. */
void json_free(json_reader *reader);

#ifdef __cplusplus
}
#endif

#endif /* JSON_H_ */
