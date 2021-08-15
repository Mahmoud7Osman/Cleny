/* MIT License
 *
 * Copyright (c) 2019 Jude Melton-Houghton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef LIBCTF_H_
#define LIBCTF_H_

/* CTF_TEST(name, ...) makes a test function with the given name and a body
 * formed of the remaining arguments. This will only be present with the symbol
 * CTF_TESTS_ENABLED defined. The body fails if it crashes or returns nonzero.
 * If it has no return statement and does not crash, it will still succeed. Make
 * sure that you are exporting symbols by default in the compiled executable, as
 * the test symbols must be present for the test runner to pick up on the tests.
 */
#if CTF_TESTS_ENABLED
#	define CTF_TEST(name, ...) \
	/* Call the function name from the test hook so that assertion failures
	 * report that function name: */ \
	static int name(void) { __VA_ARGS__; return 0; } \
	CTF_EXTERN_C CTF_EXPORT int CTF_CIRCUMFIX(name)(void) { return name(); }
#else
#	define CTF_TEST(name, ...)
#endif

/* The part of a test symbol before the identifier. */
#define CTF_PREFIX CT9f6
/* The part of a test symbol after the identifier. */
#define CTF_SUFFIX Ct5F_

/* All that follows is not a public interface. */

#define CTF_CIRCUMFIX(name) CTF_CIRCUMFIX_(CTF_PREFIX, name, CTF_SUFFIX)
#define CTF_CIRCUMFIX_(pfx, name, sfx) CTF_CIRCUMFIX_2(pfx, name, sfx)
#define CTF_CIRCUMFIX_2(pfx, name, sfx) pfx##name##sfx
#ifdef __cplusplus
#	define CTF_EXTERN_C extern "C"
#else
#	define CTF_EXTERN_C
#endif
/* To help protect against LTO when possible: */
#ifdef __GNUC__
#	define CTF_EXPORT __attribute__((used))
#else
#	define CTF_EXPORT
#endif

#endif /* LIBCTF_H_ */
