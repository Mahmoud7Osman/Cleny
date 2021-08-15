This is my JSON parser. I want some practice making parsers, and JSON seemed
relatively easy and useful.

## API
The API is documented in `json.h`, but here's a quick example:
```c
/* Compile with -DJSON_WITH_STDIO */

FILE *file = ...;
char buf[BUFSIZ];
json_reader rdr;
struct json_item item;

json_alloc(&rdr, NULL, 8, malloc, free, realloc);
json_source_file(&rdr, buf, sizeof(buf), file);
do {
	if (json_read_item(&rdr, &item) < 0) {
		/* Handle the error, the type of which is in item.type. */
	} else {
		/* Append this item using item.type, item.key, and item.val. */
	}
} while (item.type != JSON_EMPTY);
```

Here's the same example using zlib GZip decompression:
```c
#include <zlib.h>

static int refill_gz(char **buf, size_t *size, void *ctx)
{
	gzFile file = ctx;
	int got = gzread(file, *buf, *size);
	if (got < 0) return -JSON_ERROR_IO;
	if ((size_t)got < *size) {
		*size = got;
		return 0;
	}
	return 1;
}

gzFile file = ...;
char buf[BUFSIZ];
json_reader rdr;
struct json_item item;

json_alloc(&rdr, NULL, 8, malloc, free, realloc);
json_source(&rdr, buf, sizeof(buf), file, refill_gz);
do {
	if (json_read_item(&rdr, &item) < 0) {
		/* Handle the error, the type of which is in item.type. */
	} else {
		/* Append this item using item.type, item.key, and item.val. */
	}
} while (item.type != JSON_EMPTY);
```

As you can see, all it takes is changing a callback to add another stage to the
pipeline.

For a more in-depth example, see `parse.c`.

This library reads and writes UTF-8, but you can easily adapt it to other
encodings if you wish because of the flexibility of the input source.

## Tests
I got the tests from [this repository](https://github.com/nst/JSONTestSuite),
which can be found under the `tests` directory here, or the `test_parsing`
directory in the original project. Many thanks to Nicolas Seriot for providing
those tests publicly.

### Test Failures
The currently failing tests do not seem very important to me to fix. If the user
wants this parser to be compliant, it is easy to keep it that way. Just parse
only one map/array and check that the input source is empty afterward.

## Dependencies
Currently the library depends on the standard library for string manipulation
and character classification. It depends on the standard math library for the
pow function. It does not require the user to use the standard library
allocation, although that is the easiest method.

The tests require Perl 5 or so to run, and I haven't tested the scripts on
non-unix systems. run-tests specifically makes heavy use of POSIX system calls.
