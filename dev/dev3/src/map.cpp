#include "map.h"
#include "ent.h"
#include "grow.h"
#include "json.h"
#include "json-util.h"
#include "load-texture.h"
#include "loader.h"
#include "logger.h"
#include "string.h"
#include "util.h"
#include "xalloc.h"
#include <errno.h>
#include <tgmath.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DIRECTION NOTE
// --------------
// When loading the board, the 0th row in the JSON data is the northernmost row
// of the board. To make the angles of the camera an whatnot work out, this
// means the 0th row in the JSON is the last row of the board object and vice
// versa. This comment is referenced in relevant parts of the code.

static uint8_t *get_wall(struct map *map, size_t x, size_t y)
{
	return &map->walls[y * d3d_board_width(map->board) + x];
}

static uint8_t get_wall_ck(const struct map *map, long lx, long ly)
{
	size_t x = (size_t)lx, y = (size_t)ly;
	if (x < d3d_board_width(map->board) && y < d3d_board_height(map->board))
	{
		return *get_wall((struct map *)map, x, y);
	} else {
		return 0;
	}
}

static void parse_block(d3d_block_s *block, uint8_t *wall, struct json_node *nd,
	struct loader *ldr)
{
	union json_node_data *got;
	*wall = 0;
	memset(block, 0, sizeof(*block));
	if (nd->kind != JN_MAP) return;
	bool all_solid = false;
	if ((got = json_map_get(nd, "all_solid", JN_BOOLEAN))) {
		if (got->boolean) {
			*wall = 0xFF;
			all_solid = true;
		}
	}
	if ((got = json_map_get(nd, "all", JN_STRING))) {
		const d3d_texture *txtr = load_texture(ldr, got->str);
		if (txtr) {
			block->faces[D3D_DPOSX] =
			block->faces[D3D_DPOSY] =
			block->faces[D3D_DNEGX] =
			block->faces[D3D_DNEGY] =
			block->faces[D3D_DUP] =
			block->faces[D3D_DDOWN] = txtr;
		}
	}
	struct { const char *txtr, *wall; d3d_direction dir; } faces[] = {
		{"north" , "north_solid", D3D_DPOSY}, // See DIRECTION NOTE.
		{"south" , "south_solid", D3D_DNEGY},
		{"east"  , "east_solid" , D3D_DPOSX},
		{"west"  , "west_solid" , D3D_DNEGX},
		{"top"   , "" /* N/A */ , D3D_DUP  },
		{"bottom", "" /* N/A */ , D3D_DDOWN}
	};
	for (size_t i = 0; i < ARRSIZE(faces); ++i) {
		if ((got = json_map_get(nd, faces[i].txtr, JN_STRING))) {
			const d3d_texture *txtr = load_texture(ldr, got->str);
			if (txtr) block->faces[faces[i].dir] = txtr;
		}
		if (!all_solid
		 && (got = json_map_get(nd, faces[i].wall, JN_BOOLEAN)))
			if (got->boolean) *wall |= 1 << faces[i].dir;
	}
}

void map_check_walls(struct map *map, d3d_vec_s *pos, d3d_scalar radius)
{
	/* Yes, I know this code repeats itself a lot. */
	// Tile coordinates:
	long x = floor(pos->x), y = floor(pos->y);
	// xs (east, west) and ys (south, north) of 8 tiles around (x, y):
	long west, south, east, north;
	west = floor(pos->x - radius);
	south = floor(pos->y - radius);
	east = floor(pos->x + radius);
	north = floor(pos->y + radius);
	// The wall bits here at (x, y):
	uint8_t here = get_wall_ck(map, x, y);
	if (south < y) {
		// Correct the south side and maybe east/west.
		bool south_corrected = bitat(here, D3D_DNEGY);
		d3d_scalar south_dist = pos->y - floor(pos->y);
		if (south_corrected) pos->y = y + radius;
		if (west < x) {
			// Correct the west.
			if (bitat(here, D3D_DNEGX)) {
				pos->x = x + radius;
			} else if (!south_corrected) {
				// Detect southwest corner collision.
				uint8_t nw = get_wall_ck(map, west, south);
				bool hit_outer_corner = bitat(nw, D3D_DPOSY)
					|| bitat(nw, D3D_DPOSX);
				if (hit_outer_corner) {
					d3d_scalar west_dist = pos->x
						- floor(pos->x);
					// Do correction needing less movement.
					if (south_dist > west_dist) {
						pos->y = y + radius;
					} else {
						pos->x = x + radius;
					}
				}
			}
		} else if (east > x) {
			// Correct the east.
			if (bitat(here, D3D_DPOSX)) {
				pos->x = x + 1.0 - radius;
			} else if (!south_corrected) {
				// Detect southeast corner collision.
				uint8_t ne = get_wall_ck(map, east, south);
				bool hit_outer_corner = bitat(ne, D3D_DPOSY)
					|| bitat(ne, D3D_DNEGX);
				if (hit_outer_corner) {
					// Do correction needing less movement.
					d3d_scalar east_dist = ceil(pos->x)
						- pos->x;
					if (south_dist > east_dist) {
						pos->y = y + radius;
					} else {
						pos->x = x + 1.0 - radius;
					}
				}
			}
		}
	} else if (north > y) {
		// Correct the north side and maybe east/west.
		bool north_corrected = bitat(here, D3D_DPOSY);
		d3d_scalar north_dist = ceil(pos->y) - pos->y;
		if (north_corrected) pos->y = y + 1.0 - radius;
		if (west < x) {
			// Correct the west.
			if (bitat(here, D3D_DNEGX)) {
				pos->x = x + radius;
			} else if (!north_corrected) {
				// Detect northwest corner collision.
				uint8_t sw = get_wall_ck(map, west, north);
				bool hit_outer_corner = bitat(sw, D3D_DNEGY)
					|| bitat(sw, D3D_DPOSX);
				if (hit_outer_corner) {
					d3d_scalar west_dist = pos->x
						- floor(pos->x);
					// Do correction needing less movement.
					if (north_dist > west_dist) {
						pos->y = y + 1.0 - radius;
					} else {
						pos->x = x + radius;
					}
				}
			}
		} else if (east > x) {
			// Correct the east.
			if (bitat(here, D3D_DPOSX)) {
				pos->x = x + 1.0 - radius;
			} else if (!north_corrected) {
				// Detect northeast corner collision.
				uint8_t se = get_wall_ck(map, east, north);
				bool hit_outer_corner = bitat(se, D3D_DNEGY)
					|| bitat(se, D3D_DNEGX);
				if (hit_outer_corner) {
					d3d_scalar east_dist = ceil(pos->x)
						- pos->x;
					if (north_dist > east_dist) {
						pos->y = y + 1.0 - radius;
					} else {
						pos->x = x + 1.0 - radius;
					}
				}
			}
		}
	} else if (west < x) {
		// Correct only the west side.
		if (bitat(here, D3D_DNEGX)) pos->x = x + radius;
	} else if (east > x) {
		// Correct only the east side.
		if (bitat(here, D3D_DPOSX)) pos->x = x + 1.0 - radius;
	}
}

static int parse_ent_start(struct map_ent_start *start, struct loader *ldr,
	struct json_node *root)
{
	union json_node_data *got;
	start->frame = 0;
	if ((got = json_map_get(root, "kind", JN_STRING))) {
		struct ent_type *kind = load_ent_type(ldr, got->str);
		if (!kind) goto error_kind;
		start->type = kind;
	} else {
		goto error_kind;
	}
	start->team = TEAM_UNALIGNED;
	if (start->type->team_override != TEAM_INVALID) {
		start->team = start->type->team_override;
	} else if ((got = json_map_get(root, "team", JN_STRING))) {
		enum team team = team_from_str(got->str);
		if (team == TEAM_INVALID) {
			logger_printf(loader_logger(ldr), LOGGER_WARNING,
				"Invalid entity team: \"%s\"\n", got->str);
		} else {
			start->team = team;
		}
	}
	start->pos.x = start->pos.y = 0;
	if ((got = json_map_get(root, "pos", JN_LIST)))
		parse_json_vec(&start->pos, &got->list);
	return 0;

error_kind:
	logger_printf(loader_logger(ldr), LOGGER_ERROR,
		"Entity start specification has no \"kind\" attribute\n");
	return -1;
}

static uint8_t normalize_wall(const struct map *map, size_t x, size_t y)
{
	uint8_t here = map->walls[y * d3d_board_width(map->board) + x];
	d3d_direction dirs[] = {D3D_DNEGY, D3D_DPOSY, D3D_DNEGX, D3D_DPOSX};
	for (size_t i = 0; i < ARRSIZE(dirs); ++i) {
		d3d_direction dir = dirs[i];
		uint8_t bit = 1 << dir;
		if (here & bit) break;
		size_t there_x = x, there_y = y;
		move_direction(dir, &there_x, &there_y);
		if (d3d_board_get(map->board, there_x, there_y)) {
			uint8_t there =
				map->walls[there_y * d3d_board_width(map->board)
				+ there_x];
			if (bitat(there, flip_direction(dir))) here |= bit;
		} else {
			here |= bit;
		}
	}
	return here;
}

char *map_prereq(struct loader *ldr, const char *name)
{
	char *path = loader_map_path(ldr, name);
	FILE *file = fopen(path, "r");
	if (!file) {
		free(path);
		return NULL;
	}
	json_reader rdr;
	json_alloc(&rdr, NULL, 1, xmalloc, free, xrealloc);
	char buf[BUFSIZ];
	json_source_file(&rdr, buf, sizeof(buf), file);
	struct json_item item;
	scan_json_key(&rdr, "prereq", &item);
	json_free(&rdr);
	fclose(file);
	free(item.key.bytes);
	free(path);
	return item.type == JSON_STRING ? item.val.str.bytes : NULL;
}

struct map *load_map(struct loader *ldr, const char *name)
{
	struct logger *log = loader_logger(ldr);
	FILE *file;
	struct map **mapp = loader_map(ldr, name, &file);
	if (!mapp) return NULL;
	struct map *map = *mapp;
	if (map) return map;
	map = xmalloc(sizeof(*map));
	struct json_node jtree;
	map->name = str_dup(name);
	map->prereq = NULL;
	map->board = NULL;
	map->walls = NULL;
	map->blocks = NULL;
	map->ents = NULL;
	map->n_ents = 0;
	if (parse_json_tree(name, file, log, &jtree)) goto parse_error;
	if (jtree.kind != JN_MAP) {
		if (jtree.kind != JN_ERROR)
			logger_printf(log, LOGGER_ERROR,
				"Map \"%s\" is not a JSON dictionary\n", name);
		goto format_error;
	}
	void **player_nodep;
	if ((player_nodep = table_get(&jtree.d.map, "player"))) {
		if (parse_ent_start(&map->player, ldr, *player_nodep)) {
			logger_printf(log, LOGGER_ERROR,
				"Map \"%s\" has invalid \"player\" attribute\n",
				name);
			goto format_error;
		}
	} else {
		logger_printf(log, LOGGER_ERROR,
			"Map \"%s\" has no \"player\" attribute\n", name);
		goto format_error;
	}
	union json_node_data *got;
	if ((got = json_map_get(&jtree, "name", TAKE_NODE | JN_STRING))) {
		free(map->name);
		map->name = got->str;
	}
	if ((got = json_map_get(&jtree, "prereq", TAKE_NODE | JN_STRING)))
		map->prereq = got->str;
	uint8_t *walls = NULL;
	size_t n_blocks = 0;
	if ((got = json_map_get(&jtree, "blocks", JN_LIST))) {
		n_blocks = got->list.n_vals;
		walls = xmalloc(n_blocks);
		map->blocks = xmalloc(n_blocks * sizeof(*map->blocks));
		for (size_t i = 0; i < n_blocks; ++i) {
			parse_block(&map->blocks[i], &walls[i],
				&got->list.vals[i], ldr);
		}
	}
	struct json_node_data_list *layout = NULL;
	if ((got = json_map_get(&jtree, "layout", JN_LIST)))
		layout = &got->list;
	size_t width = 0, height = 0;
	if (layout) {
		height = layout->n_vals;
		for (size_t y = 0; y < height; ++y) {
			if (layout->vals[y].kind != JN_LIST) continue;
			struct json_node_data_list *row =
				&layout->vals[y].d.list;
			if (row->n_vals > width) width = row->n_vals;
		}
	}
	if (layout && width > 0 && height > 0) {
		map->board = assert_alloc(d3d_new_board(width, height, NULL));
		map->walls = xcalloc(height, width);
		// See DIRECTION NOTE regarding y:
		for (size_t r = 0, y = height - 1; r < height; ++r, --y) {
			if (layout->vals[y].kind != JN_LIST) continue;
			struct json_node_data_list *row =
				&layout->vals[r].d.list;
			for (size_t x = 0; x < row->n_vals; ++x) {
				if (row->vals[x].kind != JN_NUMBER) continue;
				size_t idx = row->vals[x].d.num;
				if (idx >= n_blocks) continue;
				*d3d_board_get(map->board, x, y) =
					&map->blocks[idx];
				*get_wall(map, x, y) = walls[idx];
			}
		}
		for (size_t y = 0; y < height; ++y) {
			for (size_t x = 0; x < width; ++x) {
				*get_wall(map, x, y) =
					normalize_wall(map, x, y);
			}
		}
	} else {
		width = height = 1;
		map->board = assert_alloc(d3d_new_board(width, height, NULL));
		map->walls = xcalloc(1, 1);
	}
	free(walls);
	map->player.pos.x = CLAMP(map->player.pos.x, 0, width - 0.01);
	// See DIRECTION NOTE:
	map->player.pos.y = height - CLAMP(map->player.pos.y, 0.01, height);
	map->n_ents = 0;
	if ((got = json_map_get(&jtree, "ents", JN_LIST))) {
		map->ents = xmalloc(got->list.n_vals * sizeof(*map->ents));
		for (size_t i = 0; i < got->list.n_vals; ++i) {
			struct map_ent_start *start = &map->ents[map->n_ents];
			if (!parse_ent_start(start, ldr, &got->list.vals[i])) {
				// See DIRECTION NOTE:
				start->pos.y = height - start->pos.y;
				++map->n_ents;
			}
		}
	}
	if (!map->board) map->board = assert_alloc(d3d_new_board(0, 0, NULL));
	free_json_tree(&jtree);
	*mapp = map;
	return map;

format_error:
	free_json_tree(&jtree);
	map_free(map);
parse_error:
	return NULL;
}

void map_free(struct map *map)
{
	if (!map) return;
	free(map->name);
	free(map->prereq);
	d3d_free_board(map->board);
	free(map->walls);
	free(map->blocks);
	free(map->ents);
	free(map);
}
