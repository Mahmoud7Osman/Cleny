#if CTF_TESTS_ENABLED

#include "test-file.h"
#include "xalloc.h"
#include <assert.h>
#include <unistd.h>

FILE *test_input(const char *mem, size_t size)
{
	int pipe_fds[2];
	assert(!pipe(pipe_fds));
	ssize_t w = write(pipe_fds[1], mem, size);
	assert(w >= 0 && (size_t)w == size);
	assert(!close(pipe_fds[1]));
	FILE *in = fdopen(pipe_fds[0], "r");
	assert(in);
	return in;
}

FILE *test_output(int *read_fd)
{
	int pipe_fds[2];
	assert(!pipe(pipe_fds));
	*read_fd = pipe_fds[0];
	FILE *out = fdopen(pipe_fds[1], "w");
	assert(out);
	return out;
}

void test_read_output(int read_fd, char **buf, size_t *size)
{
	const size_t max_size = 2048;
	*buf = xmalloc(max_size);
	ssize_t r = read(read_fd, *buf, max_size);
	assert(r >= 0);
	*size = (size_t)r;
	assert(!close(read_fd));
}

#endif /* CTF_TESTS_ENABLED */

// Make sure compilation unit is non-empty to comply with ISO C.
char test_unused_var;
