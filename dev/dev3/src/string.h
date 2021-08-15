#ifndef STRING_H_
#define STRING_H_

#include <stddef.h>

// A string of text (not necessarily NUL-terminated) with a given length. The
// text may or may not be allocated.
struct string {
	size_t len;
	char *text;
};

// Allocate a string with the given capacity and zero length.
void string_init(struct string *str, size_t cap);

// Grow a string by a given number of characters. cap, which may be modified,
// must be the allocation size of the text. String length increases. Returned is
// a pointer to the new, uninitialized memory at the end of the string.
char *string_grow(struct string *str, size_t *cap, size_t num);

// Shrink the size of a string's allocation to match its length.
void string_shrink_to_fit(struct string *str);

// Place npush characters from push at the end of the string with the given
// capacity (which may change.)
void string_pushn(struct string *str, size_t *cap, const char *push,
	size_t npush);

// Place characters before NUL from push at the end of the string with the given
// capacity (which may change.)
void string_pushz(struct string *str, size_t *cap, const char *push);

// Push a single byte push at the end of the string.
void string_pushc(struct string *str, size_t *cap, int push);

#endif /* STRING_H_ */
