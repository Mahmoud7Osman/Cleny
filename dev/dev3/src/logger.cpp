#include "logger.h"
#include <stdarg.h>
#include <unistd.h>

// Private flags for logger.flags.
#define LOGGER_COLOR		0x0010
#define LOGGER_NO_COLOR		0x0020

static int get_mode(int flags)
{
	if (flags & LOGGER_INFO) {
		return LOGGER_INFO;
	} else if (flags & LOGGER_WARNING) {
		return LOGGER_WARNING;
	} else if (flags & LOGGER_ERROR) {
		return LOGGER_ERROR;
	}
	return LOGGER_INFO;
}

static FILE **get_filep(struct logger *log, int mode)
{
	switch (mode) {
	default:
	case LOGGER_INFO:
		return &log->info;
	case LOGGER_WARNING:
		return &log->warning;
	case LOGGER_ERROR:
		return &log->error;
	}
}

static const char *get_prefix(struct logger *log, int mode)
{
	(void)log;
	switch (mode) {
	default:
	case LOGGER_INFO:
		return "[INFO] ";
	case LOGGER_WARNING:
		return "[WARNING] ";
	case LOGGER_ERROR:
		return "[ERROR] ";
	}
}

void logger_init(struct logger *log)
{
	if (!log) return;
	log->flags = 0;
	log->info = log->warning = log->error = NULL;
	log->do_free = 0;
}

FILE *logger_get_output(struct logger *log, int which)
{
	if (!log) return NULL;
	return *get_filep(log, get_mode(which));
}

void logger_set_output(struct logger *log, int which, FILE *dest, bool do_free)
{
	if (!log) return;
	if ((which & LOGGER_ALL) == LOGGER_ALL) {
		log->do_free &= ~LOGGER_ALL;
		log->flags &= ~LOGGER_ALL;
		log->info = log->warning = log->error = dest;
		if (dest) {
			log->flags |= LOGGER_ALL;
			if (do_free) log->do_free |= LOGGER_ALL;
		}
	} else {
		int mode = get_mode(which);
		log->do_free &= ~mode;
		*get_filep(log, mode) = dest;
		if (dest) {
			log->flags |= mode;
			if (do_free) log->do_free |= mode;
		} else {
			log->flags &= ~mode;
		}
	}
}
void logger_printf(struct logger *log, int flags, const char *format, ...)
{
	if (!log) return;
	int mode = get_mode(flags);
	if (!(log->flags & mode)) return;
	FILE *file = *get_filep(log, mode);
	fputs(get_prefix(log, mode), file);
	va_list args;
	va_start(args, format);
	vfprintf(file, format, args);
	va_end(args);
	fflush(file);
}

void logger_free(struct logger *log)
{
	if (!log) return;
	// Make sure no files are closed twice.
	FILE *filei = NULL, *filew = NULL, *filee = NULL;
	if ((log->do_free & LOGGER_INFO)
	 && (filei = *get_filep(log, LOGGER_INFO)))
		fclose(filei);
	if ((log->do_free & LOGGER_WARNING)
	 && (filew = *get_filep(log, LOGGER_WARNING))
	 && !(filew == filei && log->do_free & LOGGER_INFO))
		fclose(filew);
	if ((log->do_free & LOGGER_ERROR)
	 && (filee = *get_filep(log, LOGGER_ERROR))
	 && !(filee == filei && log->do_free & LOGGER_INFO)
	 && !(filee == filew && log->do_free & LOGGER_WARNING))
		fclose(filee);
}
