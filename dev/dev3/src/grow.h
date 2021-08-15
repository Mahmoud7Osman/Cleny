#ifndef GROW_H_
#define GROW_H_

#include <stddef.h>

// Grow an element array (*bufp) if size *lenp and allocation size *capp. The
// elements are all of size esize. Returned is a pointer to the newly allocated
// element.
void *growe(void **bufp, size_t *lenp, size_t *capp, size_t esize);

// Shortcut to avoid having to reference arguments to growe or to specify the
// size of the elements. Each argument is evaluated once. Return value is like
// growe.
#define GROWE(buf, len, cap) \
	growe((void **)&(buf), &(len), &(cap), sizeof *(buf))

// Grow a character buffer (*bufp) of size *lenp and allocation size *capp by
// the number of characters num. Returned is a pointer to the memory added to
// the end.
char *growc(char **bufp, size_t *lenp, size_t *capp, size_t num);

#endif /* GROW_H_ */
