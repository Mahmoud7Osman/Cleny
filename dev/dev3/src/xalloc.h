#ifndef XALLOC_H_
#define XALLOC_H_

#include <stdlib.h>

// Memory allocation alternatives that abort on failure, never returning NULL.

void *xmalloc(size_t);

void *xcalloc(size_t, size_t);

void *xrealloc(void *, size_t);

// If ptr is NULL, print a message and abort. If not, ptr is just returned.
void *assert_alloc(void *ptr);

#endif /* XALLOC_H_ */
