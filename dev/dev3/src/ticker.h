#ifndef TICKER_H_
#define TICKER_H_

#ifndef _WIN32

#	include <sys/time.h>

struct ticker {
	struct timeval last_tick;
	long interval;
};

#else

#	include <stdint.h>

struct ticker {
	uint32_t last_tick;
	uint32_t interval;
};

#endif /* defined(_WIN32) */

// Initialize a given ticker with interval in milliseconds from 1 to 999.
void ticker_init(struct ticker *tkr, int interval);

// Go forward a tick, trying to account for time spent computing inbetween.
void tick(struct ticker *tkr);

#endif /* TICKER_H_ */
