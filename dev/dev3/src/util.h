#ifndef UTIL_H_
#define UTIL_H_

#include "d3d.h"
#include <stdbool.h>
#include <stddef.h>

// Version of GNU __attribute__ which expands to nothing if attributes aren't
// supported. Only single parentheses are used.
#ifdef __GNUC__
#	define ATTRIBUTE(...) __attribute__((__VA_ARGS__))
#else
#	define ATTRIBUTE(...)
#endif

// Duplicate a NUL-terminated string, including the terminator, returning an
// allocated copy.
char *str_dup(const char *str);

// Turn a horizontal direction 180 degrees, or swap between up and down.
d3d_direction flip_direction(d3d_direction dir);

// Concat parts 1 and 2 with mid inbetween. Return the concatenated.
// NOTE: recalculates lengths, not efficient.
char *mid_cat(const char *part1, int mid, const char *part2);

// Ensure a file exists or create it if it doesn't exist. dir tells whether a
// directory should be searched for/created. 0 indicates success and -1 is for
// failure (with errno being set.)
int ensure_file(const char *path, bool dir);

// Allocate and return the default path of some file. name is the file name. env
// is the name an environment variable override would have; it is checked first.
// The path of the file itself is not checked for existence, but the directory
// path is unless the path was provided by the environment variable.
char *default_file(const char *name, const char *env);

// Call setenv() with the same arguments if it is supported, or fail otherwise.
int try_setenv(const char *name, const char *value, int overwrite);

// Substitute '/' for the native separator character in the path.
void subst_native_dir_sep(char *path);

// Move x OR y in the direction dir. Underflow in x or y is NOT accounted for.
void move_direction(d3d_direction dir, size_t *x, size_t *y);

// Normalizes the vector to the given magnitude, which may be positive or
// negative. If the vector is zero, it is unaffected.
void vec_norm_mul(d3d_vec_s *vec, d3d_scalar mag);

// Get the bit in bits at the index idx, starting from the least significant.
// Zero or one is returned.
#define bitat(bits, idx) ((bits) >> (idx) & 1)

// Number of elements in an array whose size and type is known at compile time.
#define ARRSIZE(array) (sizeof(array) / sizeof *(array))

// Clamp num within the range [min, max]. Return the clamped value. Arguments
// will be evaluated multiple times.
// Precondition: min <= max
#define CLAMP(num, min, max) \
	((num) < (min) ? (min) : ((num) > (max) ? (max) : (num)))

// The constant pi.
#define PI 3.14159265358979323846

// Convert the expression to a string after substitution. For example,
// STRINGIFY(PI) would become "3.1415...".
#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x

// Wrap this around the name of a variable at its point of definition to
// suppress warnings by the compiler that it is unused.
#define UNUSED_VAR(var) var ATTRIBUTE(unused)

#ifdef _WIN32
#	define DIRSEP '\\'
#else
#	define DIRSEP '/'
#endif

#endif /* UTIL_H_ */
