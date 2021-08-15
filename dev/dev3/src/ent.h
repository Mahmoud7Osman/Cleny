#ifndef ENTITY_H_
#define ENTITY_H_

#include "chance.h"
#include "d3d.h"
#include "team.h"
#include <stdbool.h>

struct loader; // Weak dependency

struct ent_frame {
	// The texture displayed.
	const d3d_texture *txtr;
	// Time where frame is visible, in game ticks.
	long duration;
};

struct ent_type {
	// The allocated type name.
	char *name;
	// The width of the entity in blocks.
	d3d_scalar width;
	// The height of the entity in blocks.
	d3d_scalar height;
	// The number of frames.
	size_t n_frames;
	// The allocated list of n_frames frames.
	struct ent_frame *frames;
	// entity to spawn on death.
	struct ent_type *death_spawn;
	// entity to act as a bullet.
	struct ent_type *bullet;
	// Ticks to stay alive, or -1 for forever.
	long lifetime;
	// Whether or not the animation should start at a random frame.
	bool random_start_frame;
	// Chance that the entity will turn toward the player in any given tick.
	chance turn_chance;
	// Chance that the entity will shoot in any given tick.
	chance shoot_chance;
	// Movement speed (in blocks/tick).
	d3d_scalar speed;
	// Whether the entity is blocked by walls.
	bool wall_block;
	// Whether the entity dies when it hits a wall.
	bool wall_die;
	// Team of this entity overriding the team in a map's entity start.
	enum team team_override;
	// The starting health for entities of this type.
	double health;
	// The damage to others on contact for entities of this type.
	double damage;
};

// Load an entity with the name or use one previously loaded.
// Allocate the entity.
struct ent_type *load_ent_type(struct loader *ldr, const char *name);

// Free a loaded entity type. Does nothing when given NULL.
void ent_type_free(struct ent_type *ent);

// A set of entities
struct ent;
struct ents {
	// Per-entity data.
	struct ent *ents;
	// Sprite data, parallel to ents.
	d3d_sprite_s *sprites;
	// The number of entities.
	size_t num;
	// The memory capacity of the ents and sprites buffers.
	size_t cap;
};

// A number representing an entity. This is valid from when it is returned by
// add_ents to when ents_clean_up_dead is called.
typedef size_t ent_id;

// Initialize the entity set with the given capacity.
void ents_init(struct ents *ents, size_t cap);

// Get the current number of entities in the set.
size_t ents_num(const struct ents *ents);

// Return the sprite buffer of the set. Valid until another ents_* function is
// called.
d3d_sprite_s *ents_sprites(struct ents *ents);

// Add an entity to the set with the given type and position. Its id, valid
// until ents_clean_up_dead is called, is returned.
ent_id ents_add(struct ents *ents, struct ent_type *type, enum team team,
	const d3d_vec_s *pos);

// A loop header to go through each entity id in ents. A variable var is created
// and updated with an ID for each iteration. Don't call ents_clean_up_dead
// while looping.
#define ENTS_FOR_EACH(ents, var) for (ent_id var = 0; var < ents->num; ++var)

// A loop header to go through each possible pair of entities in ents. The loop
// variables var1 and var2 are defined for each iteration as the two ids. Since
// this uses nested loop magic, don't use break or continue inside it.
#define ENTS_FOR_EACH_PAIR(ents, var1, var2) \
	for (ent_id var1 = 0; var1 < ents->num; ++var1) \
		for (ent_id var2 = var1 + 1; var2 < ents->num; ++var2)

// Perform lifecycle stuff for each entity. Kills old entities. Doesn't move
// anything.
void ents_tick(struct ents *ents);

// Determines whether an entity with the given ID is dead.
bool ents_is_dead(struct ents *ents, ent_id eid);

// Gets a pointer to entity position of entity with ID eid. Valid until ents_add
// or any function that modifies all entities is called. This is an alias for
// &ents_body(ents, eid)->pos
d3d_vec_s *ents_pos(struct ents *ents, ent_id eid);

// Gets a pointer to entity body of entity with ID eid. Valid until ents_add or
// any function that modifies all entities is called.
struct body *ents_body(struct ents *ents, ent_id eid);

// Same as ents_pos, but for velocity.
d3d_vec_s *ents_vel(struct ents *ents, ent_id eid);

// Get a pointer to the desired entity's type, valid until ents_clean_up_dead is
// called.
struct ent_type *ents_type(struct ents *ents, ent_id eid);

// Get the team of the desired entity.
enum team ents_team(struct ents *ents, ent_id eid);

// Kill an entity so that it will be removed when ent_clean_up_dead is called.
void ents_kill(struct ents *ents, ent_id eid);

// Get a pointer to the worth of the enemy. Valid until ents_add or any function
// that modifies all entities is called. The worth is higher if it is more
// important that the enemy be killed. The scale is up to the user. The only two
// worths currently used are 1 if the entity must die for the level to be done
// or 0 otherwise.
int *ents_worth(struct ents *ents, ent_id eid);

// Remove memory for dead entities. May shuffle entity IDs.
void ents_clean_up_dead(struct ents *ents);

// Deallocate memory for an entity set.
void ents_destroy(struct ents *ents);

#endif /* ENTITY_H_ */
