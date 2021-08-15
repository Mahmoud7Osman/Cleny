#define D3D_USE_INTERNAL_STRUCTS
#ifdef D3D_HEADER_INCLUDE
#	include D3D_HEADER_INCLUDE
#else
#	include "d3d.h"
#endif
#include <tgmath.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Adds the amount to the size variable, returning NULL from the current
// function on overflow.
#define CHECKED_ADD(size, amount) do { \
	size_t new_size = (size) + (amount); \
	if (new_size < (size)) return NULL; \
	(size) = new_size; \
} while (0)

// Gets the memory alignment of the given type.
#define ALIGNOF(type) offsetof(struct { char c; type t; }, t)

// Increments size until a value of type could be stored at the offset size from
// an aligned pointer. Returns NULL from the current function on overflow.
#define ALIGN_SIZE(size, type) do { \
	size_t new_size = \
		((size) + ALIGNOF(type) - 1) / ALIGNOF(type) * ALIGNOF(type); \
	if (new_size < (size)) return NULL; \
	(size) = new_size; \
} while (0)


// get a pointer to a coordinate in a grid. A grid is any structure with members
// 'width' and 'height' and another member 'member' which is a buffer containing
// all items in column-major organization. Parameters may be evaluated multiple
// times. If either coordinate is outside the range, NULL is returned.
#define GET(grid, member, x, y) ((size_t)(x) < (grid)->width && (size_t)(y) < \
		(grid)->height ? \
	&(grid)->member[((size_t)(y) + (grid)->height * (size_t)(x))] : NULL)

#define PI ((d3d_scalar)3.14159265358979323846)

#ifndef D3D_CUSTOM_ALLOCATOR
void *d3d_malloc(size_t size)
{
	return malloc(size);
}

void *d3d_realloc(void *mem, size_t size)
{
	return realloc(mem, size);
}

void d3d_free(void *mem)
{
	free(mem);
}
#endif

static d3d_pixel camera_empty_pixel(d3d_camera *cam)
{
	return cam->blank_block.faces[0]->pixels[0];
}

static void empty_camera_pixels(d3d_camera *cam)
{
	d3d_pixel empty_pixel = camera_empty_pixel(cam);
	for (size_t i = 0; i < cam->width * cam->height; ++i) {
		cam->pixels[i] = empty_pixel;
	}
}

static size_t texture_size(size_t width, size_t height)
{
	size_t pixels_size = width * height * sizeof(d3d_pixel);
	if (pixels_size / sizeof(d3d_pixel) / width != height) return 0;
	size_t base = offsetof(d3d_texture, pixels);
	size_t size = base + pixels_size;
	if (base > size) return 0;
	return size;
}

// Computes fmod(n, 1.0) if n >= 0, or 1.0 + fmod(n, 1.0) if n < 0
// Returns in the range [0, 1)
static d3d_scalar mod1(d3d_scalar n)
{
	return n - floor(n);
}

// computes approximately 1.0 - fmod(n, 1.0) if n >= 0 or -fmod(n, 1.0) if n < 0
// Returns in the range [0, 1)
static d3d_scalar revmod1(d3d_scalar n)
{
	return ceil(n) - n;
}

// Compute the difference between the two angles (each themselves between 0 and
// 2π.) The result is between -π and π.
static d3d_scalar angle_diff(d3d_scalar a1, d3d_scalar a2)
{
	d3d_scalar diff = a1 - a2;
	if (diff > PI) return -2 * PI + diff;
	if (diff < -PI) return -diff - 2 * PI;
	return diff;
}

// This is pretty much floor(c). However, when c is a nonzero whole number and
// positive is true (that is, the relevant component of the delta position is
// over 0,) c is decremented and returned. This is for calculating the tile
// coordinates of a moving point hitting a wall. If it hits a wall going in the
// positive direction, it would be put in the tile one over except for the
// decrement.
static size_t tocoord(d3d_scalar c, bool positive)
{
	d3d_scalar f = floor(c);
	if (positive && f == c && c != (d3d_scalar)0.0) return (size_t)c - 1;
	return f;
}

// Move the given coordinates in the direction given. If the direction is not
// horizontal, nothing happens. No bounds are checked.
static void move_dir(d3d_direction dir, size_t *x, size_t *y)
{
	switch(dir) {
	case D3D_DPOSX: ++*x; break;
	case D3D_DPOSY: ++*y; break;
	case D3D_DNEGX: --*x; break;
	case D3D_DNEGY: --*y; break;
	default: break;
	}
}

// Rotate a horizontal direction 180°. If the direction is not horizontal, it is
// returned unmodified.
static d3d_direction invert_dir(d3d_direction dir)
{
	switch(dir) {
	case D3D_DPOSX: return D3D_DNEGX;
	case D3D_DPOSY: return D3D_DNEGY;
	case D3D_DNEGX: return D3D_DPOSX;
	case D3D_DNEGY: return D3D_DPOSY;
	default: return dir;
	}
}

d3d_camera *d3d_new_camera(
	d3d_scalar fovx,
	d3d_scalar fovy,
	size_t width,
	size_t height,
	d3d_pixel empty_pixel)
{
	size_t size;
	size_t pixels_size, txtr_offset, tans_offset, dists_offset;
	d3d_texture *empty_txtr;
	d3d_camera *cam;
	size = offsetof(d3d_camera, pixels);
	pixels_size = width * height * sizeof(d3d_pixel);
	if (width != 0 && pixels_size / sizeof(d3d_pixel) / width != height)
		return NULL;
	CHECKED_ADD(size, pixels_size);
	// This type probably has the alignment of a one-pixel texture:
	typedef struct { size_t width, height; d3d_pixel pixels[1]; } one_pix;
	ALIGN_SIZE(size, one_pix);
	txtr_offset = size;
	CHECKED_ADD(size, texture_size(1, 1));
	ALIGN_SIZE(size, d3d_scalar);
	tans_offset = size;
	if (height * sizeof(d3d_scalar) / sizeof(d3d_scalar) != height)
		return NULL;
	CHECKED_ADD(size, height * sizeof(d3d_scalar));
	dists_offset = size;
	if (width * sizeof(d3d_scalar) / sizeof(d3d_scalar) != width)
		return NULL;
	CHECKED_ADD(size, width * sizeof(d3d_scalar));
	cam = d3d_malloc(size);
	if (!cam) return NULL;
	// The members 'tans' and 'dists' are actually pointers to parts of the
	// same allocation. 'empty_txtr' is glued on before them, but is not a
	// member.
	empty_txtr = (void *)((char *)cam + txtr_offset);
	cam->tans = (void *)((char *)cam + tans_offset);
	cam->dists = (void *)((char *)cam + dists_offset);
	// Just do basic protection against non-positive FOVs as they might
	// cause issues. I could do something better than silently clamping, but
	// what do you expect a non-positive FOV to do anyway?
	cam->fov.x = fovx > (d3d_scalar)0.0 ? fovx : 0.001;
	cam->fov.y = fovy > (d3d_scalar)0.0 ? fovy : 0.001;
	cam->width = width;
	cam->height = height;
	empty_txtr->width = 1;
	empty_txtr->height = 1;
	empty_txtr->pixels[0] = empty_pixel;
	cam->blank_block.faces[D3D_DPOSX] =
	cam->blank_block.faces[D3D_DPOSY] =
	cam->blank_block.faces[D3D_DNEGX] =
	cam->blank_block.faces[D3D_DNEGY] =
	cam->blank_block.faces[D3D_DUP] =
	cam->blank_block.faces[D3D_DDOWN] = empty_txtr;
	cam->order = NULL;
	cam->order_buf_cap = 0;
	cam->last_sprites = NULL;
	cam->last_n_sprites = 0;
	empty_camera_pixels(cam);
	for (size_t y = 0; y < height; ++y) {
		d3d_scalar angle =
			cam->fov.y * ((d3d_scalar)0.5 - (d3d_scalar)y / height);
		cam->tans[y] = tan(angle);
	}
	return cam;
}

void d3d_free_camera(d3d_camera *cam)
{
	if (!cam) return;
	d3d_free(cam->order);
	// 'tans' and 'dists' are freed here too:
	d3d_free(cam);
}

d3d_texture *d3d_new_texture(size_t width, size_t height, d3d_pixel fill)
{
	if (width < 1) width = 1;
	if (height < 1) height = 1;
	size_t size = texture_size(width, height);
	if (size == 0) return NULL;
	d3d_texture *txtr = d3d_malloc(size);
	if (!txtr) return NULL;
	txtr->width = width;
	txtr->height = height;
	for (size_t i = 0; i < width * height; ++i) {
		txtr->pixels[i] = fill;
	}
	return txtr;
}

d3d_board *d3d_new_board(size_t width, size_t height, const d3d_block_s *fill)
{
	size_t size = offsetof(d3d_board, blocks);
	size_t blocks_size = width * height * sizeof(d3d_block_s *);
	if (width != 0 && blocks_size / sizeof(d3d_block_s *) / width != height)
		return NULL;
	CHECKED_ADD(size, blocks_size);
	d3d_board *board = d3d_malloc(size);
	if (!board) return NULL;
	board->width = width;
	board->height = height;
	static const d3d_block_s empty_block = {{ NULL, NULL, NULL, NULL,
		NULL, NULL }};
	if (!fill) fill = &empty_block;
	for (size_t i = 0; i < width * height; ++i) {
		board->blocks[i] = fill;
	}
	return board;
}

// Move the position pos by dpos (delta position) until a wall is hit. dir is
// set to the face of the block which was hit. If no block is hit, NULL is
// returned.
static const d3d_block_s *hit_wall(
	const d3d_board *board,
	d3d_vec_s *pos,
	const d3d_vec_s *dpos,
	d3d_direction *dir,
	const d3d_texture **txtr)
{
	const d3d_block_s *block;
	for (;;) {
		size_t x, y;
		d3d_direction inverted;
		const d3d_block_s * const *blk = NULL;
		d3d_vec_s tonext = {0, 0};
		d3d_direction y_dir = D3D_DNEGY, x_dir = D3D_DNEGX;
		if (dpos->x < (d3d_scalar)0.0) {
			// The ray is going in the -x direction.
			tonext.x = -mod1(pos->x);
		} else if (dpos->x > (d3d_scalar)0.0) {
			// The ray is going in the +x direction.
			x_dir = D3D_DPOSX;
			tonext.x = revmod1(pos->x);
		}
		if (dpos->y < (d3d_scalar)0.0) {
			// The ray is going in the -y direction.
			tonext.y = -mod1(pos->y);
		} else if (dpos->y > (d3d_scalar)0.0) {
			// They ray is going in the +y direction.
			y_dir = D3D_DPOSY;
			tonext.y = revmod1(pos->y);
		}
		if (dpos->x == (d3d_scalar)0.0) {
			goto hit_y;
		} else if (dpos->y == (d3d_scalar)0.0) {
			goto hit_x;
		}
		if (tonext.x / dpos->x < tonext.y / dpos->y) {
			// The ray will hit a wall on the x-axis first
	hit_x:
			*dir = x_dir;
			pos->x += tonext.x;
			pos->y += tonext.x / dpos->x * dpos->y;
		} else {
			// The ray will hit a wall on the y-axis first
	hit_y:
			*dir = y_dir;
			pos->y += tonext.y;
			pos->x += tonext.y / dpos->y * dpos->x;
		}
		x = tocoord(pos->x, dpos->x > (d3d_scalar)0.0);
		y = tocoord(pos->y, dpos->y > (d3d_scalar)0.0);
		inverted = invert_dir(*dir);
		blk = GET(board, blocks, x, y);
		if (!blk) return NULL; // The ray left the board
		if ((*blk)->faces[*dir]) {
			block = *blk;
			*txtr = block->faces[*dir];
			*dir = inverted;
			return block;
		} else {
			// The face the ray hit is empty
			move_dir(*dir, &x, &y);
			blk = GET(board, blocks, x, y);
			if (!blk) return NULL; // The ray left the board
			if ((*blk)->faces[inverted]) {
				block = *blk;
				*txtr = block->faces[inverted];
				return block;
			} else {
				// The face the ray hit is empty
				// Nudge the ray past the wall:
				if (*dir == x_dir) {
					pos->x += copysign((d3d_scalar)0.0001,
						dpos->x);
				} else {
					pos->y += copysign((d3d_scalar)0.0001,
						dpos->y);
				}
			}
		}
		block = *blk;
	}
}

static void draw_column(
	d3d_camera *cam,
	d3d_vec_s cam_pos,
	d3d_scalar cam_facing,
	const d3d_board *board,
	size_t x)
{
	d3d_direction face;
	const d3d_block_s *block;
	const d3d_texture *drawing;
	d3d_vec_s pos = cam_pos, disp;
	d3d_scalar dist;
	d3d_scalar angle = cam_facing
		+ cam->fov.x * ((d3d_scalar)0.5 - (d3d_scalar)x / cam->width);
	d3d_vec_s dpos = {
		cos(angle) * (d3d_scalar)0.001, sin(angle) * (d3d_scalar)0.001
	};
	block = hit_wall(board, &pos, &dpos, &face, &drawing);
	if (!block) {
		block = &cam->blank_block;
		drawing = block->faces[0];
	}
	disp.x = pos.x - cam_pos.x;
	disp.y = pos.y - cam_pos.y;
	dist = sqrt(disp.x * disp.x + disp.y * disp.y);
	cam->dists[x] = dist;
	// Choose how far across the wall to get pixels from based on the wall
	// orientation, and put the distance in dimension:
	d3d_scalar dimension;
	switch (face) {
	case D3D_DPOSX:
		dimension = revmod1(pos.y);
		break;
	case D3D_DPOSY:
		dimension = mod1(pos.x);
		break;
	case D3D_DNEGX:
		dimension = mod1(pos.y);
		break;
	case D3D_DNEGY:
	default: // The default case shouldn't be reached.
		dimension = revmod1(pos.x);
		break;
	}
	for (size_t t = 0; t < cam->height; ++t) {
		const d3d_texture *txtr;
		size_t tx, ty;
		// The distance the ray travelled, assuming it hit a vertical
		// wall:
		d3d_scalar dist_y = cam->tans[t] * dist + (d3d_scalar)0.5;
		if (dist_y > (d3d_scalar)0.0 && dist_y < (d3d_scalar)1.0) {
			// A vertical wall was indeed hit
			txtr = drawing;
			tx = dimension * txtr->width;
			ty = txtr->height * ((d3d_scalar)1.0 - dist_y);
		} else {
			// A floor or ceiling was hit instead. The horizontal
			// displacement is adjusted accordingly:
			d3d_scalar newdist =
				(d3d_scalar)0.5 / fabs(cam->tans[t]);
			d3d_vec_s newpos = {
				cam_pos.x + disp.x / dist * newdist,
				cam_pos.y + disp.y / dist * newdist
			};
			size_t bx = newpos.x, by = newpos.y;
			const d3d_block_s *const *top_bot =
				GET(board, blocks, bx, by);
			if (!top_bot) goto no_texture;
			// Ceiling hit if dist_y >= 1, floor hit if
			// dist_y <= 0, and nothing else is possible:
			d3d_direction face =
				dist_y >= (d3d_scalar)1.0 ? D3D_DUP : D3D_DDOWN;
			txtr = (*top_bot)->faces[face];
			if (!txtr) goto no_texture;
			tx = mod1(newpos.x) * txtr->width;
			ty = mod1(newpos.y) * txtr->height;
		}
		const d3d_pixel *tpp = GET(txtr, pixels, tx, ty);
		d3d_pixel tp;
		if (tpp) {
			tp = *tpp;
		} else {
	no_texture:
			tp = camera_empty_pixel(cam);
		}
		*GET(cam, pixels, x, t) = tp;
	}
}

// Compare the sprite orders (see below). This is meant for qsort.
static int compar_sprite_order(const void *a, const void *b)
{
	d3d_scalar dist_a = *(d3d_scalar *)a, dist_b = *(d3d_scalar *)b;
	if (dist_a > dist_b) return 1;
	if (dist_a < dist_b) return -1;
	return 0;
}

static void draw_sprite_dist(
	d3d_camera *cam,
	d3d_vec_s cam_pos,
	d3d_scalar cam_facing,
	const d3d_sprite_s *sp,
	d3d_scalar dist)
{
	if (sp->scale.x <= (d3d_scalar)0.0 || sp->scale.y <= (d3d_scalar)0.0)
		return;
	d3d_vec_s disp = { sp->pos.x - cam_pos.x, sp->pos.y - cam_pos.y };
	d3d_scalar angle, width, height, diff, maxdiff;
	long start_x, start_y;
	if (dist == (d3d_scalar)0.0) return;
	// The angle of the sprite relative to the +x axis:
	angle = atan2(disp.y, disp.x);
	// The view width of the sprite in radians:
	width = atan(sp->scale.x / dist) * 2;
	// The max camera-sprite angle difference so the sprite's visible:
	maxdiff = (cam->fov.x + width) / 2;
	diff = angle_diff(cam_facing, angle);
	if (fabs(diff) > maxdiff) return;
	// The height of the sprite in pixels on the camera screen:
	height = atan(sp->scale.y / dist) * 2 / cam->fov.y * cam->height;
	// The width of the sprite in pixels on the camera screen:
	width = width / cam->fov.x * cam->width;
	// The first x where the sprite appears on the screen:
	start_x = (cam->width - width) / 2 + diff / cam->fov.x * cam->width;
	// The first y where the sprite appears on the screen:
	start_y = (cam->height - height) / 2;
	for (size_t x = 0; x < width; ++x) {
		// cx is the x on the camera screen; sx is the x on the sprite's
		// texture:
		size_t cx, sx;
		cx = x + start_x;
		if (cx >= cam->width || dist >= cam->dists[cx]) continue;
		sx = (d3d_scalar)x / width * sp->txtr->width;
		if (sx >= sp->txtr->width) continue;
		for (size_t y = 0; y < height; ++y) {
			// cy and sy correspond to cx and sx above:
			size_t cy, sy;
			cy = y + start_y;
			if (cy >= cam->height) continue;
			sy = (d3d_scalar)y / height * sp->txtr->height;
			if (sy >= sp->txtr->height) continue;
			d3d_pixel p = *GET(sp->txtr, pixels, sx, sy);
			if (p != sp->transparent) *GET(cam, pixels, cx, cy) = p;
		}
	}
}

static void draw_sprites(
	d3d_camera *cam,
	d3d_vec_s cam_pos,
	d3d_scalar cam_facing,
	size_t n_sprites,
	const d3d_sprite_s sprites[])
{
	size_t i;
	if (n_sprites == cam->last_n_sprites && sprites == cam->last_sprites) {
		// This assumes the sprites didn't move much, and are mostly
		// sorted. Therefore, insertion sort is used.
		for (i = 0; i < n_sprites; ++i) {
			size_t move_to;
			struct d3d_sprite_order ord = cam->order[i];
			size_t s = ord.index;
			ord.dist = hypot(sprites[s].pos.x - cam_pos.x,
				sprites[s].pos.y - cam_pos.y);
			move_to = i;
			for (long j = (long)i - 1; j >= 0; --j) {
				if (cam->order[j].dist <= ord.dist) break;
				move_to = j;
			}
			memmove(cam->order + move_to + 1, cam->order + move_to,
				(i - move_to) * sizeof(*cam->order));
			cam->order[move_to] = ord;
	       }
	} else {
		if (n_sprites > cam->order_buf_cap) {
			struct d3d_sprite_order *new_order;
			size_t size = n_sprites * sizeof(*cam->order);
			if (size / sizeof(*cam->order) == n_sprites
			 && (new_order = d3d_realloc(cam->order, size))) {
				cam->order = new_order;
				cam->order_buf_cap = n_sprites;
			} else {
				// XXX Silently truncate the list of sprites
				// drawn. This may be a bad decision, but
				// failure is unlikely and this shouldn't break
				// any client code.
				n_sprites = cam->order_buf_cap;
			}
		}
		for (i = 0; i < n_sprites; ++i) {
			struct d3d_sprite_order *ord = &cam->order[i];
			ord->dist = hypot(sprites[i].pos.x - cam_pos.x,
				sprites[i].pos.y - cam_pos.y);
			ord->index = i;
		}
		qsort(cam->order, n_sprites, sizeof(*cam->order),
			compar_sprite_order);
		cam->last_sprites = sprites;
		cam->last_n_sprites = n_sprites;
	}
	i = n_sprites;
	while (i--) {
		struct d3d_sprite_order *ord = &cam->order[i];
		draw_sprite_dist(cam, cam_pos, cam_facing, &sprites[ord->index],
			ord->dist);
	}
}

void d3d_draw(
	d3d_camera *cam,
	d3d_vec_s cam_pos,
	d3d_scalar cam_facing,
	const d3d_board *board,
	size_t n_sprites,
	const d3d_sprite_s sprites[])
{
	if (cam_pos.x > (d3d_scalar)0.0 && cam_pos.y > (d3d_scalar)0.0
	 && cam_pos.x < board->width && cam_pos.y < board->height) {
		// Canonicalize camera direction:
		cam_facing = fmod(cam_facing, 2 * PI);
		if (cam_facing < (d3d_scalar)0.0) cam_facing += 2 * PI;
		for (size_t x = 0; x < cam->width; ++x) {
			draw_column(cam, cam_pos, cam_facing, board, x);
		}
		draw_sprites(cam, cam_pos, cam_facing, n_sprites, sprites);
	} else {
		empty_camera_pixels(cam);
	}
}

size_t d3d_camera_width(const d3d_camera *cam)
{
	return cam->width;
}

size_t d3d_camera_height(const d3d_camera *cam)
{
	return cam->height;
}

d3d_pixel *d3d_camera_get(d3d_camera *cam, size_t x, size_t y)
{
	return GET(cam, pixels, x, y);
}

size_t d3d_texture_width(const d3d_texture *txtr)
{
	return txtr->width;
}

size_t d3d_texture_height(const d3d_texture *txtr)
{
	return txtr->height;
}

d3d_pixel *d3d_texture_get(d3d_texture *txtr, size_t x, size_t y)
{
	return GET(txtr, pixels, x, y);
}

void d3d_free_texture(d3d_texture *txtr)
{
	d3d_free(txtr);
}

size_t d3d_board_width(const d3d_board *board)
{
	return board->width;
}

size_t d3d_board_height(const d3d_board *board)
{
	return board->height;
}

const d3d_block_s **d3d_board_get(d3d_board *board, size_t x, size_t y)
{
	return GET(board, blocks, x, y);
}

void d3d_free_board(d3d_board *board)
{
	d3d_free(board);
}
