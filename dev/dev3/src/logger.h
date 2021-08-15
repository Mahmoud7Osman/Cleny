#ifndef LOGGER_H_
#define LOGGER_H_

#include "util.h"
#include <stdbool.h>
#include <stdio.h>

struct logger {
	int flags;
	// Log destinations.
	FILE *info, *warning, *error;
	// Mask of INFO, WARNING, and ERROR telling which to fclose on free.
	int do_free;
};

// Flags specifying different log levels.
#define LOGGER_INFO		0x0001
#define LOGGER_WARNING		0x0002
#define LOGGER_ERROR		0x0004
#define LOGGER_ALL		(LOGGER_INFO | LOGGER_WARNING | LOGGER_ERROR)

/*
 * All the following functions accept NULL as a log in which case they do
 * nothing. logger_get_output returns NULL.
 */

// Initialize a logger with nothing being logged (all outputs NULL.)
void logger_init(struct logger *log);

// Get the current output for the log type `which` (INFO, WARNING, or ERROR). If
// the log goes nowhere (output is discarded), NULL is returned. LOGGER_ALL is
// not valid because only one file can be returned.
FILE *logger_get_output(struct logger *log, int which);

// Set an output for `which`, which is INFO, WARNING, or ERROR. If `dest` is
// NULL, messages are never printed. If not and do_free is true, the output will
// be closed when logger_free is called. The previous output is not closed. Here
// LOGGER_ALL is a valid which, and sets the information for all levels.
void logger_set_output(struct logger *log, int which, FILE *dest, bool do_free);

// Print a formatted message. `flags` contains one of the log types, and cannot
// be LOGGER_ALL.
void logger_printf(struct logger *log, int flags, const char *format, ...)
	ATTRIBUTE(format(printf, 3, 4));

// Free a logger and close its outputs which have been marked as such.
void logger_free(struct logger *log);

#endif /* LOGGER_H_ */
