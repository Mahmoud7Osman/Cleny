#include "read-lines.h"
#include "grow.h"
#include "xalloc.h"
#include <errno.h>
#include <stdbool.h>
#include <string.h>

struct string *read_lines(FILE *file, size_t *nlines)
{
	int errnum;
	size_t lines_cap;
	struct string *lines;
	*nlines = 0;
	lines_cap = 16;
	lines = xmalloc(lines_cap * sizeof(*lines));
	char buf[BUFSIZ];
	size_t nread = sizeof(buf);
	char *head = buf + nread;
	errno = 0;
	do {
		size_t line_cap = 16;
		struct string *line = GROWE(lines, *nlines, lines_cap);
		line->len = 0;
		line->text = xmalloc(line_cap);
		for (bool last_one = false; !last_one;) {
			if (head >= buf + nread) {
				nread = fread(buf, 1, sizeof(buf), file);
				last_one = nread < sizeof(buf);
				head = buf;
			}
			char *nl = memchr(head, '\n', nread + buf - head);
			if (nl) {
				last_one = true;
			} else if (head == buf + nread
			        && line->len == 0
				&& feof(file))
			{
				--*nlines;
				free(line->text);
				goto done;
			} else {
				nl = buf + nread;
			}
			char *added = string_grow(line, &line_cap, nl - head);
			memcpy(added, head, nl - head);
			head = nl + 1;
		}
		if (line->len != 0) string_shrink_to_fit(line);
	} while (!errno && !(feof(file) && head >= buf + nread));
done:
	if (errno) {
		for (size_t i = 0; i < *nlines; ++i) {
			free(lines[i].text);
		}
		free(lines);
		lines = NULL;
	}
	errnum = errno;
	fclose(file);
	errno = errnum;
	return lines;
}

#if CTF_TESTS_ENABLED

#	include "libctf.h"
#	include "test-file.h"
#	include <assert.h>
#	include <unistd.h>

static void dump_line(const struct string *lines, unsigned long i)
{
	printf("%lu: (length %lu) %.*s\n", i + 1, (long unsigned)lines[i].len,
		(int)lines[i].len, lines[i].text);
}

CTF_TEST(read_lines_final_newline,
	char str[] = "a\nb\nc\n";
	FILE *source = test_input(str, 6);
	size_t nlines;
	struct string *lines = read_lines(source, &nlines);
	assert(nlines == 3);
	for (unsigned long i = 0; i < nlines; ++i) {
		dump_line(lines, i);
		assert(lines[i].len == 1);
		assert(*lines[i].text == str[i * 2]);
	}
)

CTF_TEST(read_lines_no_final_newline,
	char str[] = "a\nb\nc";
	FILE *source = test_input(str, 5);
	size_t nlines;
	struct string *lines = read_lines(source, &nlines);
	assert(nlines == 3);
	for (unsigned long i = 0; i < nlines; ++i) {
		dump_line(lines, i);
		assert(lines[i].len == 1);
		assert(*lines[i].text == str[i * 2]);
	}
)

CTF_TEST(read_lines_empty,
	FILE *source = test_input("", 0);
	size_t nlines;
	read_lines(source, &nlines);
	assert(nlines == 0);
)

CTF_TEST(read_lines_just_newline,
	char str[] = "\n";
	FILE *source = test_input(str, 1);
	size_t nlines;
	struct string *lines = read_lines(source, &nlines);
	assert(nlines == 1);
	assert(lines[0].len == 0);
)

#endif /* CTF_TESTS_ENABLED */
