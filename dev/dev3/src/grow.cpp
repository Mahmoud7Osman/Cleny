#include "grow.h"
#include "xalloc.h"

void *growe(void **bufp, size_t *lenp, size_t *capp, size_t esize)
{
	if (*lenp >= *capp) {
		*capp = *lenp + *lenp / 2 + 1;
		*bufp = xrealloc(*bufp, *capp * esize);
	}
	return (char *)*bufp + (*lenp)++ * esize;
}

char *growc(char **bufp, size_t *lenp, size_t *capp, size_t num)
{
	size_t old_len = *lenp;
	*lenp += num;
	if (*lenp > *capp) {
		*capp = *lenp + *lenp / 2 + 1;
		*bufp = xrealloc(*bufp, *capp);
	}
	return *bufp + old_len;
}

#if CTF_TESTS_ENABLED

#	include "libctf.h"
#	include <assert.h>

CTF_TEST(growe_from_null,
	size_t len = 0, cap = 0;
	int *buf = NULL;
	int *item0 = GROWE(buf, len, cap);
	assert(item0 == buf);
	assert(len == 1);
	assert(cap >= 1);
)

CTF_TEST(growe_realloc,
	size_t len = 0, cap = 2;
	int *buf = malloc(cap * sizeof(*buf));
	int *item;
	item = GROWE(buf, len, cap);
	assert(item == buf + 0);
	assert(len == 1);
	assert(cap == 2);
	item = GROWE(buf, len, cap);
	assert(item == buf + 1);
	assert(len == 2);
	assert(cap == 2);
	item = GROWE(buf, len, cap);
	assert(item == buf + 2);
	assert(len == 3);
	assert(cap >= 3);
)

// growc tested in string.c

#endif /* CTF_TESTS_ENABLED */
