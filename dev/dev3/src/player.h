#ifndef PLAYER_H_
#define PLAYER_H_

#include "body.h"
#include "d3d.h"

// Weak dependencies
struct ents;
struct map;
struct map_ent_start;

// Player in-game information.
struct player {
	// The player's physical body.
	struct body body;
	// The starting information of the player.
	struct map_ent_start *start;
	// The direction faced, in radians, like d3d_camera_facing.
	d3d_scalar facing;
	// The turn speed in radians per tick.
	d3d_scalar turn_speed;
	// The reload status, incremented each tick.
	long reload;
	// The status which must be reached to shoot.
	long reload_ready;
};

// Initialize a player. The player now has a dependency on the map.
void player_init(struct player *player, struct map *map);

// Get a number from 0 to 1 representing the fullness of health.
double player_health_fraction(const struct player *player);

// Get a number from 0 to 1 representing the reloadedness.
double player_reload_fraction(const struct player *player);

// Register that a tick has passed.
void player_tick(struct player *player);

// Walk in the angle relative to the direction currently faced.
void player_walk(struct player *player, d3d_scalar angle);

// Turn counter-clockwise.
void player_turn_ccw(struct player *player);

// Turn clockwise.
void player_turn_cw(struct player *player);

// Returns whether the player is dead.
bool player_is_dead(const struct player *player);

// Try to shoot a bullet and add it to ents. Returned is whether or not a bullet
// was shot. One will only be shot if the player has reloaded.
bool player_try_shoot(struct player *player, struct ents *ents);

// Simulate collisions with all nearby entities in ents, calculating damages.
void player_collide(struct player *player, struct ents *ents);

#endif /* PLAYER_H_ */
