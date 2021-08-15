#ifndef TEST_FILE_H_
#define TEST_FILE_H_

#include <stdio.h>

// These functions are for testing only. They use assert for error handling.
// They probably handle small amounts of text.

// Open a file reading from the memory of the size.
FILE *test_input(const char *mem, size_t size);

// Open a file for writing whose output can be read from read_fd.
FILE *test_output(int *read_fd);

// Read from the read_fd into the buffer of the size.
void test_read_output(int read_fd, char **buf, size_t *size);

#endif /* TEST_H_ */
