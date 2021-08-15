#ifndef TEAM_H_
#define TEAM_H_

#include <stdbool.h>

// A team for an entity, effecting what they can collide with. A complete table
// (collision_table) can be seen in team.h.
enum team {
	// This is not a team. It is returned when team_from_str could not
	// figure out which team to return.
	TEAM_INVALID = -1,
	// The team of entities which don't interact with anything.
	TEAM_GHOST,
	// Unaligned entities collide with everything except pickups.
	TEAM_UNALIGNED,
	// This is solely the team of the player. Players can collide with
	// anything except allies.
	TEAM_PLAYER,
	// An ally of the player.
	TEAM_ALLY,
	// The player's enemy who can collide with them and their allies.
	TEAM_ENEMY,
	// An item the player can pick up. These can only be hit by the player,
	// nothing else.
	TEAM_PICKUP,
	// This is not a team. It is the number of the teams above.
	N_TEAMS
};

// Parse a team from a string, case-insensitive. Returns TEAM_INVALID if the
// string does not represent a team.
enum team team_from_str(const char *str);

// Checks whether teams a and b can hit each other. This is commutative. Both a
// and b must be valid teams.
bool teams_can_collide(enum team a, enum team b);

#endif /* TEAM_H_ */
