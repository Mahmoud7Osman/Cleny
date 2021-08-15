#include "ent.h"
#include "body.h"
#include "grow.h"
#include "json.h"
#include "json-util.h"
#include "load-texture.h"
#include "loader.h"
#include "logger.h"
#include "pixel.h"
#include "string.h"
#include "util.h"
#include "xalloc.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ent {
	// Current velocity (in blocks/tick)
	d3d_vec_s vel;
	// The type of this entity.
	struct ent_type *type;
	// The entity's team.
	enum team team;
	// The worthiness of killing the enemy. See ents_worth.
	int worth;
	// The remaining lifetime of this entity in ticks.
	long lifetime;
	// The frame index into the array help by the type.
	size_t frame;
	// The remaining duration of the current frame.
	long frame_duration;
	// The bodily information of the entity.
	struct body body;
};


static void parse_frame(struct json_node *node, struct ent_frame *frame,
	struct loader *ldr)
{
	char *txtr_name = NULL;
	frame->duration = 1;
	switch (node->kind) {
	case JN_STRING:
		txtr_name = node->d.str;
		node->taken = true;
		break;
	case JN_LIST:
		if (node->d.list.n_vals < 1
		 || node->d.list.vals[0].kind != JN_STRING) break;
		txtr_name = node->d.list.vals[0].d.str;
		node->d.list.vals[0].taken = true;
		if (node->d.list.n_vals < 2
		 || node->d.list.vals[1].kind != JN_NUMBER) break;
		frame->duration = node->d.list.vals[1].d.num;
		break;
	default:
		break;
	}
	if (!txtr_name || !(frame->txtr = load_texture(ldr, txtr_name)))
		frame->txtr = loader_empty_texture(ldr);
	free(txtr_name);
}

struct ent_type *load_ent_type(struct loader *ldr, const char *name)
{
	struct logger *log = loader_logger(ldr);
	FILE *file;
	struct ent_type *ent, **entp;
	entp = loader_ent(ldr, name, &file);
	if (!entp) return NULL;
	ent = *entp;
	if (ent) return ent;
	ent = xmalloc(sizeof(*ent));
	struct json_node jtree;
	ent->name = str_dup(name);
	ent->frames = NULL;
	ent->n_frames = 0;
	ent->turn_chance = CHANCE_NEVER;
	ent->shoot_chance = CHANCE_NEVER;
	ent->death_spawn = NULL;
	ent->bullet = NULL;
	ent->lifetime = -1;
	ent->wall_block = true;
	ent->wall_die = false;
	ent->team_override = TEAM_INVALID;
	ent->health = 1.0;
	ent->damage = 0;
	if (parse_json_tree(name, file, log, &jtree)) return NULL;
	if (jtree.kind != JN_MAP) {
		if (jtree.kind != JN_ERROR)
			logger_printf(log, LOGGER_WARNING,
				"Entity type \"%s\" is not a JSON dictionary\n",
				name);
		*entp = ent;
		goto end;
	}
	union json_node_data *got;
	if ((got = json_map_get(&jtree, "name", TAKE_NODE | JN_STRING))) {
		free(ent->name);
		ent->name = got->str;
	}
	ent->width = 1.0;
	if ((got = json_map_get(&jtree, "width", JN_NUMBER)))
		ent->width = got->num;
	ent->height = 1.0;
	if ((got = json_map_get(&jtree, "height", JN_NUMBER)))
		ent->height = got->num;
	ent->speed = 0.0;
	if ((got = json_map_get(&jtree, "speed", JN_NUMBER)))
		ent->speed = got->num;
	if ((got = json_map_get(&jtree, "turn_chance", JN_NUMBER)))
		ent->turn_chance = chance_from_percent(got->num);
	if ((got = json_map_get(&jtree, "shoot_chance", JN_NUMBER)))
		ent->shoot_chance = chance_from_percent(got->num);
	ent->random_start_frame =
		(got = json_map_get(&jtree, "random_start_frame", JN_BOOLEAN))
		&& got->boolean;
	if ((got = json_map_get(&jtree, "frames", JN_LIST))) {
		ent->n_frames = got->list.n_vals;
		ent->frames = xmalloc(ent->n_frames * sizeof(*ent->frames));
		for (size_t i = 0; i < ent->n_frames; ++i) {
			parse_frame(&got->list.vals[i], &ent->frames[i], ldr);
		}
	}
	// To prevent infinite recursion with infinite cycles.
	*entp = ent;
	if ((got = json_map_get(&jtree, "death_spawn", JN_STRING)))
		ent->death_spawn = load_ent_type(ldr, got->str);
	if ((got = json_map_get(&jtree, "bullet", JN_STRING)))
		ent->bullet = load_ent_type(ldr, got->str);
	if ((got = json_map_get(&jtree, "lifetime", JN_NUMBER)))
		ent->lifetime = got->num;
	if ((got = json_map_get(&jtree, "wall_block", JN_BOOLEAN))
	 && !got->boolean)
		ent->wall_block = false;
	if ((got = json_map_get(&jtree, "wall_die", JN_BOOLEAN))
	 && got->boolean)
		ent->wall_die = true;
	if ((got = json_map_get(&jtree, "team_override", JN_STRING))
	 && (ent->team_override = team_from_str(got->str)) == TEAM_INVALID)
		logger_printf(log, LOGGER_WARNING,
			"Invalid team override: \"%s\"\n", got->str);
	if ((got = json_map_get(&jtree, "health", JN_NUMBER)))
		ent->health = got->num;
	if ((got = json_map_get(&jtree, "damage", JN_NUMBER)))
		ent->damage = got->num;
end:
	if (ent->n_frames == 0) {
		ent->frames = xrealloc(ent->frames, sizeof(*ent->frames));
		ent->frames[0].txtr = loader_empty_texture(ldr);
		ent->frames[0].duration = 0;
	}
	free_json_tree(&jtree);
	return ent;
}

void ent_type_free(struct ent_type *ent)
{
	if (!ent) return;
	free(ent->name);
	free(ent->frames);
	free(ent);
}

static void ent_init(struct ent *ent, struct ent_type *type, enum team team,
	d3d_sprite_s *sprite, const d3d_vec_s *pos)
{
	ent->vel.x = ent->vel.y = 0;
	ent->type = type;
	ent->team = type->team_override == TEAM_INVALID ?
		team : type->team_override;
	ent->worth = 0;
	ent->lifetime = type->lifetime;
	ent->frame = type->random_start_frame ? rand() % type->n_frames : 0;
	ent->frame_duration = type->frames[0].duration;
	ent->body.pos = *pos;
	ent->body.radius = type->width / 2;
	ent->body.health = type->health;
	ent->body.damage = type->damage;
	sprite->txtr = type->frames[0].txtr;
	sprite->transparent = TRANSPARENT_PIXEL;
	sprite->scale.x = type->width;
	sprite->scale.y = type->height;
}

static bool ent_is_dead(const struct ent *ent)
{
	return (ent->type->lifetime >= 0 && ent->lifetime < 0)
	    || ent->body.health <= 0;
}

static void ent_tick(struct ent *ent, d3d_sprite_s *sprite)
{
	--ent->lifetime;
	if (!ent_is_dead(ent)) {
		if (--ent->frame_duration <= 0) {
			if (++ent->frame >= ent->type->n_frames) ent->frame = 0;
			ent->frame_duration =
				ent->type->frames[ent->frame].duration;
			sprite->txtr = ent->type->frames[ent->frame].txtr;
		}
	} else if (ent->type->death_spawn) {
		int worth = ent->worth;
		ent_init(ent, ent->type->death_spawn, ent->team, sprite,
			&sprite->pos);
		ent->worth = worth;
	} else {
		ent->lifetime = -1;
	}
}

void ents_init(struct ents *ents, size_t cap)
{
	ents->ents = xmalloc(cap * sizeof(*ents->ents));
	ents->sprites = xmalloc(cap * sizeof(*ents->sprites));
	ents->num = 0;
	ents->cap = cap;
}

size_t ents_num(const struct ents *ents)
{
	return ents->num;
}

d3d_sprite_s *ents_sprites(struct ents *ents)
{
	for (size_t i = 0; i < ents->num; ++i) {
		ents->sprites[i].pos = ents->ents[i].body.pos;
	}
	return ents->sprites;
}

ent_id ents_add(struct ents *ents, struct ent_type *type, enum team team,
	const d3d_vec_s *pos)
{
	size_t old_num = ents->num;
	size_t old_cap = ents->cap;
	struct ent *ent = GROWE(ents->ents, ents->num, ents->cap);
	if (ents->cap != old_cap) {
		ents->sprites = xrealloc(ents->sprites, ents->cap
			* sizeof(*ents->sprites));
	}
	ent_init(ent, type, team, &ents->sprites[old_num], pos);
	return ents->num - 1;
}

void ents_tick(struct ents *ents)
{
	for (size_t i = 0; i < ents->num; ++i) {
		ent_tick(&ents->ents[i], &ents->sprites[i]);
	}
}

d3d_vec_s *ents_pos(struct ents *ents, ent_id eid)
{
	return &ents->ents[eid].body.pos;
}

struct body *ents_body(struct ents *ents, ent_id eid)
{
	return &ents->ents[eid].body;
}

d3d_vec_s *ents_vel(struct ents *ents, ent_id eid)
{
	return &ents->ents[eid].vel;
}

struct ent_type *ents_type(struct ents *ents, ent_id eid)
{
	return ents->ents[eid].type;
}

enum team ents_team(struct ents *ents, ent_id eid)
{
	return ents->ents[eid].team;
}

bool ents_is_dead(struct ents *ents, ent_id eid)
{
	return ent_is_dead(&ents->ents[eid]);
}

void ents_kill(struct ents *ents, ent_id eid)
{
	ents->ents[eid].body.health = 0;
}

int *ents_worth(struct ents *ents, ent_id eid)
{
	return &ents->ents[eid].worth;
}

void ents_clean_up_dead(struct ents *ents)
{
	for (size_t i = 0; i < ents->num; ++i) {
		struct ent *ent = &ents->ents[i];
		if (ent_is_dead(ent)) {
			--ents->num;
			*ent = ents->ents[ents->num];
			ents->sprites[i] = ents->sprites[ents->num];
		}
	}
}

void ents_destroy(struct ents *ents)
{
	free(ents->ents);
	free(ents->sprites);
}
