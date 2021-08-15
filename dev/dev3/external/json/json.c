#include "json.h"
#include <limits.h>
#include <math.h>
#include <string.h>
#ifdef JSON_WITH_FD
#include <unistd.h>
#endif

/* Get a pointer to the structure containing the member pointed to by ptr. */
#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

/* Flags for reader::flags */
#define SOURCE_DEPLETED  0x0100
#define STARTED_COMPOUND 0x0200
#define SMALL_STACK      0x0400

/* reader::alloc compatible function which always fails. */
static void *alloc_fail(size_t size)
{
	(void)size;
	return NULL;
}

/* reader::resize compatible function which always fails. */
static void *realloc_fail(void *ptr, size_t size)
{
	(void)ptr, (void)size;
	return NULL;
}

/* reader::dealloc compatible function which does nothing. */
static void dealloc_noop(void *ptr)
{
	(void)ptr;
}

int json_alloc(json_reader *reader,
	void *stack, size_t stacksiz,
	void *(*alloc)(size_t),
	void  (*dealloc)(void *),
	void *(*resize)(void *, size_t))
{
	reader->alloc = alloc ? alloc : alloc_fail;
	reader->dealloc = dealloc ? dealloc : dealloc_noop;
	reader->resize = resize ? resize : realloc_fail;
	reader->flags = 0;
	if (!stack && stacksiz <= sizeof(reader->stack.small.frames)
		/* Just in case, make sure the small size can't overflow: */
		&& (size_t)UCHAR_MAX >= sizeof(reader->stack.small.frames))
	{
		reader->flags |= SMALL_STACK;
		reader->stack.small.size = 0;
	} else {
		if (!stack && !(stack = reader->alloc(stacksiz))) return -1;
		reader->stack.big.cap = stacksiz;
		reader->stack.big.size = 0;
	}
	return 0;
}

/* reader::refill compatible function which always indicates depletion. */
static int refill_dont(char **buf, size_t *size, void *ctx)
{
	(void)buf, (void)size, (void)ctx;
	return 0;
}

void json_source(json_reader *reader,
	char *buf, size_t bufsiz, void *ctx,
	int (*refill)(char **buf, size_t *bufsiz, void *ctx))
{
	reader->ctx = ctx;
	reader->buf = buf;
	reader->bufsiz = bufsiz;
	reader->head = bufsiz;
	if (refill) {
		reader->refill = refill;
	} else {
		reader->refill = refill_dont;
		reader->flags |= SOURCE_DEPLETED;
	}
}

void json_source_string(json_reader *reader, const char *str, size_t len)
{
	reader->ctx = NULL;
	reader->buf = (char *)str;
	reader->bufsiz = len;
	reader->head = 0;
	reader->refill = refill_dont;
	reader->flags |= SOURCE_DEPLETED;
}

#ifdef JSON_WITH_STDIO
/* reader::refill compatible function which reads from a stdio file. */
static int refill_stdio(char **buf, size_t *size, void *ctx)
{
	FILE *file = ctx;
	size_t got = fread(*buf, 1, *size, file);
	if (got < *size) {
		*size = got;
		return feof(file) ? 0 : -JSON_ERROR_ERRNO;
	}
	return 1;
}

void json_source_file(json_reader *reader, char *buf, size_t bufsiz, FILE *file)
{
	json_source(reader, buf, bufsiz, file, refill_stdio);
}
#endif /* JSON_WITH_STDIO */

#ifdef JSON_WITH_FD
/* reader::refill compatible function which reads from a file descriptor. */
static int refill_fd(char **buf, size_t *size, void *ctx)
{
	/* WARNING: This is a hack! */
	int fd = container_of(buf, json_reader, buf)->fd;
	ssize_t got = read(fd, *buf, *size);
	if (got < 0) return -JSON_ERROR_ERRNO;
	if ((size_t)got < *size) {
		*size = got;
		return 0;
	}
	(void)ctx;
	return 1;
}

void json_source_fd(json_reader *reader, char *buf, size_t bufsiz, int fd)
{
	json_source(reader, buf, bufsiz, NULL, refill_fd);
	reader->fd = fd;
}
#endif /* JSON_WITH_FD */

void json_get_buf(const json_reader *reader, char **buf, size_t *bufsiz)
{
	*buf = reader->buf;
	*bufsiz = reader->bufsiz;
}

size_t json_get_num_used(const json_reader *reader)
{
	return reader->head;
}

void **json_get_ctx(json_reader *reader)
{
	return &reader->ctx;
}

void json_free(json_reader *reader)
{
	if ((reader->flags & SMALL_STACK) == 0)
		reader->dealloc(reader->stack.big.frames);
}

/* A type of frame on the stack. */
enum frame {
	/* Not actually a frame, but returned when an empty stack is popped. */
	FRAME_EMPTY,
	/* A list is being parsed. */
	FRAME_LIST,
	/* A map is being parsed. */
	FRAME_MAP
};

/* Custom versions of ctype functions, compliant to JSON. */
static int is_space(int ch)
{
	return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'
		|| ch == '\v';
}
static int is_digit(int ch)
{
	return ch >= '0' && ch <= '9';
}
static int to_digit(int ch)
{
	return ch - '0';
}
static int to_lower(int ch)
{
	return ch | 0x20;
}

/* Returns whether an error has been set. */
static int has_error(const json_reader *reader)
{
	return (reader->flags & 0xFF) != 0;
}

int json_get_last_error(const json_reader *reader,
	enum json_type *code, size_t *erridx)
{
	if (has_error(reader)) {
		if (erridx) *erridx = reader->head;
		if (code) *code = reader->flags & 0xFF;
		return 1;
	}
	return 0;
}

/* Set error indicator if it has yet to be set. */
static void set_error(json_reader *reader, enum json_type err)
{
	if (!has_error(reader)) reader->flags |= err;
}

/* Allocate using the reader's function or set error on failure. */
static void *alloc(json_reader *reader, size_t size)
{
	void *ptr = reader->alloc(size);
	if (!ptr) set_error(reader, JSON_ERROR_MEMORY);
	return ptr;
}

/* Push a byte to the memory area. len is the logical size, while cap is the
 * size of the memory area. Returns 0 on success or -1 on error (and the error
 * is indicated.) */
static int push_byte(json_reader *reader, char **bytes,
	size_t *len, size_t *cap, int ch)
{
	if (*len >= *cap) {
		size_t new_cap = *cap + *cap / 2 + 1;
		char *new_bytes = reader->resize(*bytes, new_cap);
		if (!new_bytes) {
			set_error(reader, JSON_ERROR_MEMORY);
			return -1;
		}
		*cap = new_cap;
		*bytes = new_bytes;
	}
	(*bytes)[*len] = ch;
	++*len;
	return 0;
}

/* Push several bytes to the memory area. len is the logical size, while cap is
 * the size of the memory area. Returns 0 on success or -1 on error (and the
 * error is indicated.) */
static int push_bytes(json_reader *reader, char **bytes,
	size_t *len, size_t *cap, const char *buf, size_t bufsiz)
{
	if (*len + bufsiz > *cap) {
		size_t new_cap = *len + bufsiz;
		char *new_bytes;
		new_cap += new_cap / 2;
		new_bytes = reader->resize(*bytes, new_cap);
		if (!new_bytes) {
			set_error(reader, JSON_ERROR_MEMORY);
			return -1;
		}
		*cap = new_cap;
		*bytes = new_bytes;
	}
	memcpy(*bytes + *len, buf, bufsiz);
	*len += bufsiz;
	return 0;
}

/* Push a frame to the reader's stack. */
static int push_frame(json_reader *reader, int frame)
{
	if (reader->flags & SMALL_STACK) {
		/* Temporary stack holding buffer: */
		char tmp[sizeof(reader->stack.small.frames)];
		if (reader->stack.small.size >= sizeof(tmp)) {
			/* The small stack must be embiggened. */
			memcpy(tmp, reader->stack.small.frames, sizeof(tmp));
			size_t size = sizeof(tmp) + 1;
			if (!(reader->stack.big.frames = reader->alloc(size))) {
				/* Failure to allocate; revert and report. */
				memcpy(reader->stack.small.frames, tmp,
					sizeof(tmp));
				reader->stack.small.size = sizeof(tmp);
				set_error(reader, JSON_ERROR_MEMORY);
				return -1;
			}
			memcpy(reader->stack.big.frames, tmp, sizeof(tmp));
			reader->stack.big.frames[size - 1] = frame;
			reader->stack.big.cap = size;
			reader->stack.big.size = size;
			reader->flags &= ~SMALL_STACK;
		} else {
			reader->stack.small.frames[reader->stack.small.size++] =
				frame;
		}
		return 0;
	} else {
		return push_byte(reader, &reader->stack.big.frames,
			&reader->stack.big.size, &reader->stack.big.cap, frame);
	}
}

/* Take off and return the top stack frame, or FRAME_EMPTY if the stack is
 * empty. */
static int pop_frame(json_reader *reader)
{
	if (reader->flags & SMALL_STACK) {
		if (reader->stack.small.size == 0) return FRAME_EMPTY;
		return reader->stack.small.frames[--reader->stack.small.size];
	} else {
		if (reader->stack.big.size == 0) return FRAME_EMPTY;
		return reader->stack.big.frames[--reader->stack.big.size];
	}
}

/* Return the top stack frame, or FRAME_EMPTY if the stack is empty. */
static int peek_frame(const json_reader *reader)
{
	if (reader->flags & SMALL_STACK) {
		if (reader->stack.small.size == 0) return FRAME_EMPTY;
		return reader->stack.small.frames[reader->stack.small.size - 1];
	} else {
		if (reader->stack.big.size == 0) return FRAME_EMPTY;
		return reader->stack.big.frames[reader->stack.big.size - 1];
	}
}

/* Return whether the current buffer has more to give or if it needs to be
 * refilled. */
static int is_in_range(const json_reader *reader)
{
	return reader->head < reader->bufsiz;
}

/* Refill the buffer. Returns 0 on success or -1 (with an error indicated) on
 * failure. SOURCE_DEPLETED is set if the input has no more to give. */
static int refill(json_reader *reader)
{
	size_t newsiz = reader->bufsiz;
	/* WARNING: Must refer to &reader->buf for refill_fd to work! */
	int retval = reader->refill(&reader->buf, &newsiz, reader->ctx);
	if (retval < 0) {
		set_error(reader, -retval & 0xFF);
		return -1;
	}
	if (retval == 0) reader->flags |= SOURCE_DEPLETED;
	reader->bufsiz = newsiz;
	reader->head = 0;
	return 0;
}

/* Skip whitespace (see is_space) from the current character to the first
 * non-whitespace. The buffer is refilled as many times as is necessary. Returns
 * 0 on success or -1 on failure. */
static int skip_spaces(json_reader *reader)
{
	for (;;) {
		while (is_in_range(reader)) {
			if (!is_space(reader->buf[reader->head])) return 0;
			++reader->head;
		}
		if (reader->flags & SOURCE_DEPLETED) return 0;
		if (refill(reader)) return -1;
	}
}

/* Get the next byte from the input, refilling the buffer if necessary. Returns
 * the byte past which the head just advanced on success, or -1 when there was
 * an error OR the source was depleted (check has_error.) */
static int next_char(json_reader *reader)
{
	while (!is_in_range(reader) && (reader->flags & SOURCE_DEPLETED) == 0) {
		if (refill(reader)) return -1;
	}
	if (is_in_range(reader))
		return (unsigned char)reader->buf[reader->head++];
	return -1;
}

/* After a call to next_char, use this function so that the next call to
 * next_char, or the next examination of the byte at reader->head, will return
 * the same byte as the last call did. */
static void reexamine_char(json_reader *reader)
{
	--reader->head;
}

/* Call next_char, store it in the location ch, and execute do_fail if the call
 * returned -1. do_fail need not be semicolon-terminated. */
#define NEXT_CHAR(reader, ch, do_fail) do { \
	if (((ch)  = next_char((reader))) < 0) {do_fail;} \
} while (0)

/* Read the next bufsiz chars from the reader into buf, refilling when
 * necessary. The number returned is -1 on error (and the error is set), or the
 * number of bytes read on success. The number returned will be less than bufsiz
 * if the source had less to give than bufsiz. */
static long next_chars(json_reader *reader, char *buf, size_t bufsiz)
{
	if (reader->head + bufsiz <= reader->bufsiz) {
		memcpy(buf, reader->buf + reader->head, bufsiz);
		reader->head += bufsiz;
	} else {
		size_t i;
		size_t easy_copy = reader->bufsiz - reader->head;
		memcpy(buf, reader->buf + reader->head, easy_copy);
		if (refill(reader)) return -1;
		for (i = easy_copy; i < bufsiz; ++i) {
			int ch;
			NEXT_CHAR(reader, ch, return
				reader->flags & SOURCE_DEPLETED ? (long)i : -1);
			buf[i] = ch;
		}
	}
	return bufsiz;
}

/* Parse a double-precision number according to JSON's grammar. Returns 0 on
 * success (and sets the result to a number) or -1 on error with a hopefully
 * appropriate error message. */
static int parse_number(json_reader *reader, struct json_item *result)
{
	int status = JSON_ERROR_TOKEN;
	double num = 0.0;
	double sign = 1.0;
	int ch;
	NEXT_CHAR(reader, ch, goto error);
	if (ch == '-') {
		sign = -1.0;
		NEXT_CHAR(reader, ch, goto error);
	}
	if (ch == '0') {
		status = 0;
		NEXT_CHAR(reader, ch, goto finish);
	} else if (is_digit(ch)) {
		status = 0;
		do {
			num *= 10;
			num += to_digit(ch);
			NEXT_CHAR(reader, ch, goto finish);
		} while (is_digit(ch));
	} else {
		goto error;
	}
	if (ch == '.') {
		double fraction = 0.0;
		long n_digits = 0;
		status = JSON_ERROR_NUMBER_FORMAT;
		NEXT_CHAR(reader, ch, goto error);
		if (is_digit(ch)) {
			status = 0;
			do {
				n_digits++;
				fraction *= 10;
				fraction += to_digit(ch);
				NEXT_CHAR(reader, ch,
					num += fraction / pow(10, n_digits);
					goto finish);
			} while (is_digit(ch));
		} else {
			goto error;
		}
		num += fraction / pow(10, n_digits);
	}
	if (ch == 'e' || ch == 'E') {
		long expsign = 1;
		long exponent = 0;
		status = JSON_ERROR_NUMBER_FORMAT;
		NEXT_CHAR(reader, ch, goto error);
		switch (ch) {
		case '-':
			expsign = -1;
			/* FALLTHROUGH */
		case '+':
			NEXT_CHAR(reader, ch, goto error);
			break;
		}
		while (is_digit(ch)) {
			status = 0;
			if (exponent > (LONG_MAX - 9) / 10) {
				/* Avoid undefined signed overflow. */
				status = JSON_ERROR_NUMBER_FORMAT;
				goto error;
			}
			exponent *= 10;
			exponent += to_digit(ch);
			NEXT_CHAR(reader, ch,
				num *= pow(expsign * 10, exponent);
				goto finish;
			);
		}
		num *= pow(expsign * 10, exponent);
	}
	if (status) goto error;
	reexamine_char(reader);
finish:
	num *= sign;
	result->type = JSON_NUMBER;
	result->val.num = num;
	return -has_error(reader);

error:
	reexamine_char(reader);
	set_error(reader, status);
	return -1;
}

/* Parse a single-token value, meaning null, a boolean, or a number. On success,
 * 0 is returned and the result is set. Otherwise, -1 is returned and an error
 * is set.
 * XXX THIS CURRENTLY DOES NOT CHECK IF THE READER IS IN RANGE BEFOREHAND! */
static int parse_token_value(json_reader *reader,
	struct json_item *result)
{
	long read;
	char tokbuf[5];
	switch (reader->buf[reader->head]) {
	case 't': /* true */
		if ((read = next_chars(reader, tokbuf, 4)) < 0) goto error;
		if (read < 4 || memcmp(tokbuf, "true", 4)) goto error_invalid;
		result->type = JSON_BOOLEAN;
		result->val.boolean = 1;
		break;
	case 'f': /* false */
		if ((read = next_chars(reader, tokbuf, 5)) < 0) goto error;
		if (read < 5 || memcmp(tokbuf, "false", 5)) goto error_invalid;
		result->type = JSON_BOOLEAN;
		result->val.boolean = 0;
		break;
	case 'n': /* null */
		if ((read = next_chars(reader, tokbuf, 4)) < 0) goto error;
		if (read < 4 || memcmp(tokbuf, "null", 4)) goto error_invalid;
		result->type = JSON_NULL;
		break;
	default: /* number */
		if (parse_number(reader, result)) goto error;
		break;
	}
	return 0;

error_invalid:
	set_error(reader, JSON_ERROR_TOKEN);
error:
	return -1;
}

/* Checks whether the given UTF-16 code unit is the high part of a surrogate
 * pair. */
static int is_high_surrogate(unsigned utf16)
{
	return (utf16 & 0xD800) == 0xD800;
}

/* Checks whether the given UTF-16 code unit is the low part of a surrogate
 * pair. */
static int is_low_surrogate(unsigned utf16)
{
	return (utf16 & 0xDC00) == 0xDC00;
}


/* Converts a non-paired UTF-16 code unit to a unicode codepoint. */
static long utf16_to_codepoint(unsigned utf16)
{
	return utf16;
}

/* Converts a UTF-16 surrogate pair to a unicode codepoint. */
static long utf16_pair_to_codepoint(unsigned high, unsigned low)
{
	return (high - 0xD800) * 0x400 + (low - 0xDC00) + 0x10000;
}

/* Converts a unicode codepoint (cp) to a UTF-8 code unit in buf. Returns the
 * size of the code unit in bytes. */
static size_t codepoint_to_utf8(long cp, char buf[4])
{
	if (cp <= 0x7F) {
		buf[0] = cp;
		return 1;
	} else if (cp <= 0x7FF) {
		buf[0] = 0xC0 | (cp >> 6);
		buf[1] = 0x80 | (cp & 0x3F);
		return 2;
	} else if (cp <= 0xFFFF) {
		buf[0] = 0xE0 | (cp >> 12);
		buf[1] = 0x80 | ((cp >> 6) & 0x3F);
		buf[2] = 0x80 | (cp & 0x3F);
		return 3;
	} else {
		buf[0] = 0xF0 | (cp >> 18);
		buf[1] = 0x80 | ((cp >> 12) & 0x3F);
		buf[2] = 0x80 | ((cp >> 6) & 0x3F);
		buf[3] = 0x80 | (cp & 0x3F);
		return 4;
	}
}

/* Parses an unsigned short (2 byte) number from four hexidecimal digits.
 * Returns the number on success, or -1 when a digit is invalid. This is case-
 * insensitive. */
static long hex_short(const char hex[4])
{
	long num = 0;
	unsigned shift = 0;
	long i;
	for (i = 3, shift = 0; i >= 0; --i, shift += 4) {
		int dig = to_lower(hex[i]);
		long nibble;
		if (is_digit(dig)) nibble = to_digit(dig);
		else if (dig >= 'a' && dig <= 'f') nibble = 10 + to_digit(dig);
		else return -1;
		num |= nibble << shift;
	}
	return num;
}

/* Read the part of an escape character AFTER the backslash into a string. On
 * success, zero is returned and the character(s) are pushed onto the string.
 * On failure (there was an I/O error or an invalid escape), -1 is returned. You
 * probably shouldn't rely on the contents of the string in that case.
 *
 * XXX This may read two escape characters due to the inability of the parser to
 * backtrack when parsing invalid surrogate pairs. */
static int escape_char(json_reader *reader, struct json_string *str,
	size_t *cap)
{
	/* Whether an extra hex-escaped codepoint was read which did not match
	 * the proceeding high surrogate: */
	int read_extra_cp = 0;
	/* Whether an extra non-hex escape was read: */
	int read_extra_escape = 0;
	/* The UTF-16 code units read (up to 2 are possible): */
	long utf16[2] = {-1, -1};
	/* The unicode codepoint if one was read, possibly from a surrogate
	 * pair: */
	long codepoint;
	/* The extra unpaired codepoint if it was read (-1 otherwise): */
	long extracp = -1;
	/* The UTF-8 version of a codepoint if it was read: */
	char utf8[4];
	/* The buffer of hex digits: */
	char buf[4];
	/* The amount some call to next_chars read: */
	long read;
	int ch = next_char(reader);
	if (ch < 0) goto error;
	switch (ch) {
	case 'b': ch = '\b'; break;
	case 'f': ch = '\f'; break;
	case 'n': ch = '\n'; break;
	case 'r': ch = '\r'; break;
	case 't': ch = '\t'; break;
	case '"': ch =  '"'; break;
	case'\\':/*Same as */break;
	case '/':/*escaped.*/break;
	case 'u':
		read = next_chars(reader, buf, 4);
		if (read < 4) goto error;
		utf16[0] = hex_short(buf);
		if (utf16[0] < 0) goto error;
		codepoint = utf16_to_codepoint(utf16[0]);
		if (is_high_surrogate(utf16[0])) {
			/* It wants a partner. */
			NEXT_CHAR(reader, ch, goto error);
			if (ch != '\\') {
				/* No partner; only normal chars were found: */
				reexamine_char(reader);
			} else {
				/* Escape follows. */
				NEXT_CHAR(reader, ch, goto error);
				if (ch == 'u') {
					/* Hex escape follows. */
					read = next_chars(reader, buf,
						4);
					if (read < 0) goto error;
					if (read < 4) goto error;
					utf16[1] = hex_short(buf);
					if (utf16[1] < 0) goto error;
					if (is_low_surrogate(utf16[1])) {
						/* Partner found. */
						codepoint =
							utf16_pair_to_codepoint(
							utf16[0], utf16[1]);
					} else {
						/* Extra codepoint found. */
						read_extra_cp = 1;
						extracp = utf16_to_codepoint(
								utf16[1]);
					}
				} else {
					/* Extra other escape was found, will be
					 * parsed below. (read_extra_escape) */
					reexamine_char(reader);
					read_extra_escape = 1;
				}
			}
		}
		if (push_bytes(reader, &str->bytes, &str->len, cap,
			utf8, codepoint_to_utf8(codepoint, utf8))) goto error;
		if (read_extra_cp) {
			if (push_bytes(reader, &str->bytes, &str->len, cap,
					utf8, codepoint_to_utf8(extracp, utf8)))
				goto error;
		} else if (read_extra_escape) {
			/* This will only every recurse once, since this can
			 * only occur for \uXXXX, but that is handled non-
			 * recursively. */
			if (escape_char(reader, str, cap)) goto error;
		}
		return 0;
	default:
		goto error;
	}
	if (push_byte(reader, &str->bytes, &str->len, cap, ch)) goto error;
	return 0;

error:
	set_error(reader, JSON_ERROR_ESCAPE);
	reexamine_char(reader);
	return -1;
}

/* Parse a quoted string. This DOES check that the first character is '"'. On
 * success, 0 is returned and str has been allocated. On failure, -1 is returned
 * and the error is set. The string is then freed. */
static int parse_string(json_reader *reader, struct json_string *str)
{
	int ch;
	char *oldbytes;
	size_t cap = 16;
	NEXT_CHAR(reader, ch, return -1);
	if (ch != '"') goto error_expected_string;
	str->bytes = alloc(reader, cap);
	if (!str->bytes) return -1;
	str->len = 0;
	while ((ch = next_char(reader)) != '"') {
		if (ch < 0) goto error_unclosed_quote;
		if (ch == '\\') {
			if (escape_char(reader, str, &cap))
				goto error;
		} else if (ch < 32) {
			goto error_control_char;
		} else {
			if (push_byte(reader, &str->bytes, &str->len, &cap, ch))
				goto error;
		}
	}
	if (push_byte(reader, &str->bytes, &str->len, &cap, '\0')) goto error;
	oldbytes = str->bytes;
	str->bytes = reader->resize(str->bytes, str->len);
	if (!str->bytes) str->bytes = oldbytes;
	--str->len; /* Because of the NUL terminator */
	return 0;

error_expected_string:
	set_error(reader, JSON_ERROR_EXPECTED_STRING);
	goto error;

error_unclosed_quote:
	set_error(reader, JSON_ERROR_UNCLOSED_QUOTE);
	goto error;

error_control_char:
	reexamine_char(reader);
	set_error(reader, JSON_ERROR_CONTROL_CHAR);
	goto error;

error:
	reader->dealloc(str->bytes);
	return -1;
}

/* Parse any JSON value. Compound values have only their beginnings parsed. On
 * success, 0 is returned and the result is set, while -1 is returned and an
 * error is set on failure. */
static int parse_value(json_reader *reader, struct json_item *result)
{
	int ch;
	NEXT_CHAR(reader, ch, goto error_expected_value);
	switch (ch) {
	case '[':
		push_frame(reader, FRAME_LIST);
		reader->flags |= STARTED_COMPOUND;
		result->type = JSON_LIST;
		break;
	case '{':
		push_frame(reader, FRAME_MAP);
		reader->flags |= STARTED_COMPOUND;
		result->type = JSON_MAP;
		break;
	case '"':
		reexamine_char(reader);
		if (parse_string(reader, &result->val.str)) goto error;
		result->type = JSON_STRING;
		break;
	default:
		reexamine_char(reader);
		if (parse_token_value(reader, result)) goto error;
		break;
	}
	return 0;

error_expected_value:
	if (!has_error(reader)) set_error(reader, JSON_ERROR_EXPECTED_VALUE);
error:
	return -1;
}

/* See if the next character is ench. If so, the stack is popped and the
 * compound ending result type is set. If not, the character is left to be
 * examined again. */
static int try_compound_end(json_reader *reader, int endch,
	enum json_type type, struct json_item *result)
{
	int ch;
	NEXT_CHAR(reader, ch, return -has_error(reader));
	if (ch == endch) {
		pop_frame(reader);
		result->type = type;
	} else {
		reexamine_char(reader);
	}
	return 0;
}

/* See if the next character is ench. If so, the stack is popped and the
 * compound ending result type is set. If the character is ',' instead, the
 * parser simply advances past the character. Otherwise, that's an error. On
 * success, 0 is returned, while -1 is returned and an error is set on failure.
 */
static int parse_after_elem(json_reader *reader, int endch,
	enum json_type type, struct json_item *result)
{
	int ch;
	NEXT_CHAR(reader, ch, return -has_error(reader));
	if (ch == endch) {
		pop_frame(reader);
		result->type = type;
	} else if (ch != ',') {
		set_error(reader, JSON_ERROR_BRACKETS);
		return -1;
	}
	return 0;
}

/* See if the current character is ':'. If so, the parser advances past. If not,
 * an error is set and -1 is returned (0 is returned on success.)
 * XXX THIS CURRENTLY DOES NOT CHECK IF THE READER IS IN RANGE BEFOREHAND! */
static int parse_colon(json_reader *reader)
{
	if (reader->buf[reader->head] != ':') {
		set_error(reader, JSON_ERROR_EXPECTED_COLON);
		return -1;
	}
	++reader->head;
	return 0;
}

int json_read_item(json_reader *reader, struct json_item *result)
{
	if (has_error(reader)) goto error;
	result->type = JSON_EMPTY;
	result->key.len = 0;
	result->key.bytes = NULL;
	if (!is_in_range(reader)) {
		if (reader->flags & SOURCE_DEPLETED) {
			if (((reader->flags & SMALL_STACK) != 0
					&& reader->stack.small.size == 0)
				|| ((reader->flags & SMALL_STACK) == 0
					&& reader->stack.big.size == 0))
			{
				/* All brackets have been closed. */
				return 0;
			} else {
				set_error(reader, JSON_ERROR_BRACKETS);
				goto error;
			}
		} else if (refill(reader)) {
			goto error;
		}
	}
	switch (peek_frame(reader)) {
	case FRAME_EMPTY:
		if (skip_spaces(reader)) goto error;
		if (is_in_range(reader) && parse_value(reader, result))
			goto error;
		return 0;
	case FRAME_LIST:
		if (skip_spaces(reader)) goto error;
		if (reader->flags & STARTED_COMPOUND) {
			if (try_compound_end(reader, ']', JSON_END_LIST,
				result)) goto error;
			reader->flags &= ~STARTED_COMPOUND;
		} else {
			if (parse_after_elem(reader, ']', JSON_END_LIST,
				result)) goto error;
		}
		if (result->type == JSON_END_LIST) return 0;
		(void)(
			skip_spaces(reader) ||
			parse_value(reader, result)
		);
		break;
	case FRAME_MAP:
		if (skip_spaces(reader)) goto error;
		if (reader->flags & STARTED_COMPOUND) {
			if (try_compound_end(reader, '}', JSON_END_MAP,result))
				goto error;
			reader->flags &= ~STARTED_COMPOUND;
		} else {
			if (parse_after_elem(reader, '}', JSON_END_MAP, result))
				goto error;
		}
		if (result->type == JSON_END_MAP) return 0;
		(void)(
			skip_spaces(reader) ||
			parse_string(reader, &result->key) ||
			skip_spaces(reader) ||
			parse_colon(reader) ||
			skip_spaces(reader) ||
			parse_value(reader, result)
		);
		break;
	}
	if (has_error(reader)) goto error;
	return 0;

error:
	json_get_last_error(reader, &result->type, &result->val.erridx);
	return -1;
}
