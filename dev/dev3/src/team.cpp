#include "team.h"
#include <strings.h>

enum team team_from_str(const char *str)
{
	if (!strcasecmp(str, "GHOST")) {
		return TEAM_GHOST;
	} else if (!strcasecmp(str, "UNALIGNED")) {
		return TEAM_UNALIGNED;
	} else if (!strcasecmp(str, "PLAYER")) {
		return TEAM_PLAYER;
	} else if (!strcasecmp(str, "ALLY")) {
		return TEAM_ALLY;
	} else if (!strcasecmp(str, "ENEMY")) {
		return TEAM_ENEMY;
	} else if (!strcasecmp(str, "PICKUP")) {
		return TEAM_PICKUP;
	} else {
		return TEAM_INVALID;
	}
}

bool teams_can_collide(enum team a, enum team b)
{
	static const bool collision_table[N_TEAMS][N_TEAMS] = {
	                /* GHOST  UNALIGNED PLAYER ALLY   ENEMY  PICKUP  */
	/*     GHOST */ {  false, false,    false, false, false, false   },
	/* UNALIGNED */ {  false, true,     true,  true,  true,  false   },
	/*    PLAYER */ {  false, true,     false, false, true,  true    },
	/*      ALLY */ {  false, true,     false, false, true,  false   },
	/*     ENEMY */ {  false, true,     true,  true,  false, false   },
	/*    PICKUP */ {  false, false,    true,  false, false, false   },
	};
	return collision_table[a][b];
}

#if CTF_TESTS_ENABLED
#	include "libctf.h"
#	include <assert.h>

CTF_TEST(team_collision_commutative,
	for (int a = 0; a < N_TEAMS; ++a) {
		for (int b = 0; b < N_TEAMS; ++b) {
			assert(teams_can_collide(a, b)
				== teams_can_collide(b, a));
		}
	}
)

#endif /* CTF_TESTS_ENABLED */
