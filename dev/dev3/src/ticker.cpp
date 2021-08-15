#include "ticker.h"

#ifndef _WIN32

#	include <stddef.h>
#	include <time.h>

void ticker_init(struct ticker *tkr, int interval)
{
	gettimeofday(&tkr->last_tick, NULL);
	tkr->interval = (long)interval * 1000L;
}

void tick(struct ticker *tkr)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	long elapsed = (long)now.tv_usec - (long)tkr->last_tick.tv_usec;
	if (elapsed < 0) elapsed += 1000000;
	long delay = tkr->interval - elapsed;
	if (delay > 0) {
		struct timespec delay_ts =
			{ .tv_sec = 0, .tv_nsec = delay * 1000 };
		nanosleep(&delay_ts, NULL);
	}
	gettimeofday(&tkr->last_tick, NULL);
}

#else

#	include <windows.h>

void ticker_init(struct ticker *tkr, int interval)
{
	tkr->last_tick = GetTickCount();
	tkr->interval = interval;
}

void tick(struct ticker *tkr)
{
	DWORD now = GetTickCount();
	DWORD deadline = tkr->last_tick + tkr->interval;
	if (now < deadline) {
		Sleep(deadline - now);
		tkr->last_tick = deadline;
	} else {
		tkr->last_tick = now;
	}
}

#endif /* defined(_WIN32) */

#if CTF_TESTS_ENABLED

#	include "libctf.h"
#	include <assert.h>
#	include <sys/time.h>

CTF_TEST(tick_is_right_length,
	struct ticker tkr;
	struct timeval before, after;
	struct timespec sleep = { .tv_sec = 0, .tv_nsec = 51000000 };
	gettimeofday(&before, NULL);
	ticker_init(&tkr, 500);
	nanosleep(&sleep, NULL);
	tick(&tkr);
	gettimeofday(&after, NULL);
	long delay = after.tv_usec - before.tv_usec
		+ 1000000 * (after.tv_sec - before.tv_sec);
	assert(delay > 450000);
	assert(delay < 550000);
)

CTF_TEST(much_missed_tick_time_ignored,
	struct ticker tkr;
	struct timeval before, after;
	struct timespec sleep = { .tv_sec = 0, .tv_nsec = 500000000 };
	ticker_init(&tkr, 20);
	nanosleep(&sleep, NULL);
	gettimeofday(&before, NULL);
	tick(&tkr);
	tick(&tkr);
	gettimeofday(&after, NULL);
	long delay = after.tv_usec - before.tv_usec
		+ 1000000 * (after.tv_sec - before.tv_sec);
	assert(delay > 20000);
	assert(delay < 23000);
)

#endif /* CTF_TESTS_ENABLED */
