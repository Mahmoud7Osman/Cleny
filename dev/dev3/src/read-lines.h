#ifndef READ_LINES_H_
#define READ_LINES_H_

#include "string.h"
#include <stdio.h>

// Read the lines from the file and close the file. Put the count in nlines.
// Return the list of lines.
struct string *read_lines(FILE *file, size_t *nlines);

#endif /* READ_LINES_H_ */
