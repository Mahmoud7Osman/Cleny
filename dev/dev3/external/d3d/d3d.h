#ifndef D3D_H_
#define D3D_H_

#include <stddef.h>

/* COMPILE-TIME OPTIONS
 * --------------------
 *  - D3D_PIXEL_TYPE: Pixels can be any integer type you want, but you MUST
 *    compile both your code and this library with the same D3D_PIXEL_TYPE. The
 *    types from stdint.h are not automatically available. Pixels are
 *    unsigned char by default.
 *  - D3D_SCALAR_TYPE: d3d_scalar can be any real type supported by tgmath.h,
 *    but you MUST compile both your code and this library with the same
 *    D3D_SCALAR_TYPE. Scalars are the type double by default.
 *  - D3D_CUSTOM_ALLOCATOR: Don't use malloc, realloc, and free to implement
 *    d3d_malloc, d3d_realloc, and d3d_free within d3d.c. Instead, the compiled
 *    source code must be linked with custom implementations of those three
 *    functions. You NEED NOT compile d3d.c and your code with the same setting
 *    of this option, although your code may need to provide implementations
 *    depending on the setting used with d3d.c.
 *  - D3D_USE_INTERNAL_STRUCTS: Define this to have access to unstable internal
 *    structure layouts of opaque types used by the library. You NEED NOT
 *    compile d3d.c and your code with the same setting of this option.
 *  - D3D_HEADER_INCLUDE: If this is defined, instead of '#include "d3d.h"',
 *    d3d.c will use '#include D3D_HEADER_INCLUDE'. This is ONLY useful when
 *    compiling d3d.c, not the client code. */

/* Custom allocator routines. These have the same contract of behaviour as the
 * corresponding functions in the standard library. They are meant for internal
 * use by the library. If the preprocessor symbol D3D_CUSTOM_ALLOCATOR is
 * defined, provide implementations in another compilation unit. Otherwise, the
 * library implements these with the standard library functions. Allocations
 * might fail without ever calling these functions. */
void *d3d_malloc(size_t);
void *d3d_realloc(void *, size_t);
void d3d_free(void *);

/* A single numeric pixel, for storing whatever data you provide in a
 * texture. */
#ifdef D3D_PIXEL_TYPE
typedef D3D_PIXEL_TYPE d3d_pixel;
#else
typedef unsigned char d3d_pixel;
#endif

/* A scalar. */
#ifdef D3D_SCALAR_TYPE
typedef D3D_SCALAR_TYPE d3d_scalar;
#else
typedef double d3d_scalar;
#endif

/* A vector in two dimensions. */
typedef struct {
	d3d_scalar x, y;
} d3d_vec_s;

/* A grid of pixels with certain width and height. */
struct d3d_texture_s;
typedef struct d3d_texture_s d3d_texture;

/* A block on a board consisting of 6 face textures. The indices correspond with
 * the values of d3d_direction. Vertical sides are mirrored if the viewer is
 * looking from the inside of a block outward. */
typedef struct {
	const d3d_texture *faces[6];
} d3d_block_s;

/* A two-dimensional entity with an analog position on the grid. A sprite always
 * faces toward the viewer always. Sprites are centered halfway between the top
 * and bottom of the space. */
typedef struct {
	/* The position of the sprite. */
	d3d_vec_s pos;
	/* The x and y stretch factor of the sprite. If one of these is 1.0,
	 * then the sprite is 1 tile in size in that dimension. */
	d3d_vec_s scale;
	/* The texture of the sprite. */
	const d3d_texture *txtr;
	/* A pixel value that is transparent in the texture. If this is -1, none
	 * of the pixels can be transparent. */
	d3d_pixel transparent;
} d3d_sprite_s;

/* An object representing a view into the world and what it sees. */
struct d3d_camera_s;
typedef struct d3d_camera_s d3d_camera;

/* An object representing a location for blocks, sprites, and a camera. */
struct d3d_board_s;
typedef struct d3d_board_s d3d_board;

/* A direction. The possible values are all listed below. */
typedef enum {
	D3D_DPOSX, /* Positive x direction. */
	D3D_DPOSY, /* etc. */
	D3D_DNEGX,
	D3D_DNEGY,
	D3D_DUP,
	D3D_DDOWN
} d3d_direction;

/* Allocate a new camera with given field of view in the x and y directions (in
 * radians) and a given view width and height in pixels. The empty pixel is the
 * pixel value set in the camera when a cast ray hits nothing (it gets off the
 * edge of the board.) NULL is returned if allocation fails. */
d3d_camera *d3d_new_camera(
	d3d_scalar fovx,
	d3d_scalar fovy,
	size_t width,
	size_t height,
	d3d_pixel empty_pixel);

/* Get the view width of a camera in pixels. */
size_t d3d_camera_width(const d3d_camera *cam);

/* Get the view height of a camera in pixels. */
size_t d3d_camera_height(const d3d_camera *cam);

/* Get a pixel in the camera's view. This returns NULL if the coordinates are
 * out of range. Otherwise, the pointer is valid until another function takes
 * the camera as a non-const parameter. If d3d_draw hasn't yet been called with
 * the camera, all the pixels are the camera's empty_pixel. This funcion does
 * not modify the camera in any way. */
d3d_pixel *d3d_camera_get(d3d_camera *cam, size_t x, size_t y);

/* Destroy a camera object. It shall never be used again. */
void d3d_free_camera(d3d_camera *cam);

/* Allocate a new texture with its pixels initialized to the fill pixel.  NULL
 * is returned if allocation fails. The width and height will be made 1 if they
 * are 0, as a dimension of 0 cannot be stretched across another dimension. */
d3d_texture *d3d_new_texture(size_t width, size_t height, d3d_pixel fill);

/* Get the width of the texture in pixels. */
size_t d3d_texture_width(const d3d_texture *txtr);

/* Get the height of the texture in pixels. */
size_t d3d_texture_height(const d3d_texture *txtr);

/* Get a pixel at a coordinate on a texture. NULL is returned if the coordinates
 * are out of range. The pointer is valid until the texture is used (indirectly)
 * in d3d_draw. This function does not modify the texture in any way. */
d3d_pixel *d3d_texture_get(d3d_texture *txtr, size_t x, size_t y);

/* Permanently destroy a texture. */
void d3d_free_texture(d3d_texture *txtr);

/* Create a new board with a width and height. All its blocks are initially
 * set to the block fill. If fill is NULL, all the blocks are set to an
 * empty/transparent block. NULL is returned if allocation fails. */
d3d_board *d3d_new_board(size_t width, size_t height, const d3d_block_s *fill);

/* Get the width of the board in blocks. */
size_t d3d_board_width(const d3d_board *board);

/* Get the height of the board in blocks. */
size_t d3d_board_height(const d3d_board *board);

/* Get a block in a board. If the coordinates are out of range, NULL is
 * returned. Otherwise, a pointer to a block POINTER is returned. This pointed-
 * to pointer can be modified with a new block pointer. The outer pointer is
 * valid until the board is used in d3d_draw. This function does not modify the
 * board in any way. */
const d3d_block_s **d3d_board_get(d3d_board *board, size_t x, size_t y);

/* Permanently destroy a board. */
void d3d_free_board(d3d_board *board);

/* Record what the camera sees, making it valid to access camera pixels. This
 * function is the entire point of this library.
 *
 * The camera is positioned according to cam_pos and is facing in the direction
 * cam_facing, measured in radians. In its orientation in relation to the axes,
 * the facing angle is just like an angle around the unit circle, so increasing
 * the angle turns the camera counter-clockwise. NOTE that the +y direction on
 * the board is therefore "UP" just like the +y direction of the axes on which
 * the unit circle is drawn, NOT "DOWN" like with other grids in this library.
 *
 * The given sprites are drawn inside the environment of the board. The sprites
 * pointer can be NULL if n_sprites is 0.
 *
 * Out-of-bounds coordinates are tolerated. However, it is unspecified what
 * pixels will be captured if the camera is not within the borders of the board.
 * Sprites outside the board will not be drawn. */
void d3d_draw(
	d3d_camera *cam,
	d3d_vec_s cam_pos,
	d3d_scalar cam_facing,
	const d3d_board *board,
	size_t n_sprites,
	const d3d_sprite_s sprites[]);

#endif /* D3D_H_ */

/* Complete structure definitions. */
#if defined(D3D_USE_INTERNAL_STRUCTS) && !defined(D3D_INTERNAL_H_)
#define D3D_INTERNAL_H_

struct d3d_texture_s {
	// Width and height in pixels.
	size_t width, height;
	// Column-major pixels.
	d3d_pixel pixels[];
};

// This is for drawing multiple sprites.
struct d3d_sprite_order {
	// The distance from the camera
	d3d_scalar dist;
	// The corresponding index into the given d3d_sprite_s list
	size_t index;
};

struct d3d_camera_s {
	// The field of view in the x (sideways) and y (vertical) screen axes.
	// Measured in radians.
	d3d_vec_s fov;
	// The width and height of the camera screen, in pixels.
	size_t width, height;
	// The block containing all empty textures.
	d3d_block_s blank_block;
	// The last buffer used when sorting sprites, or NULL the first time.
	struct d3d_sprite_order *order;
	// The capacity (allocation size) of the field above.
	size_t order_buf_cap;
	// The last sprites drawn.
	const d3d_sprite_s *last_sprites;
	// The number of sprites last drawn.
	size_t last_n_sprites;
	// For each row of the screen, the tangent of the angle of that row
	// relative to the center of the screen, in radians
	// For example, the 0th item is tan(fov.y / 2)
	d3d_scalar *tans;
	// For each column of the screen, the distance from the camera to the
	// first wall in that direction. This is calculated when drawing columns
	// and is used when drawing sprites.
	d3d_scalar *dists;
	// The pixels of the screen in column-major order.
	d3d_pixel pixels[];
};

struct d3d_board_s {
	// The width and height of the board, in blocks
	size_t width, height;
	// The blocks of the boards. These are pointers to save space with many
	// identical blocks.
	const d3d_block_s *blocks[];
};

#endif /* defined(D3D_USE_INTERNAL_STRUCTS) && !defined(D3D_INTERNAL_H_) */
