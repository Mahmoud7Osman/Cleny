#ifndef UI_UTIL_H_
#define UI_UTIL_H_

#include "d3d.h"
#include <curses.h>

// The ASCII escape key.
#define ESC '\033'

// The ASCII delete key.
#define DEL '\177'

#ifndef KEY_RESIZE
	// If it's not defined, set KEY_RESIZE to something that won't come up
	// often in the input. Implementations without KEY_RESIZE will simply
	// not support resizing the window.
#	define KEY_RESIZE '\a'
#endif

// A map from pixel values to Curses color pairs. The fields are private. If all
// possible pixels were initialized, Curses might run out of color pairs.
struct color_map {
	// The number of initialized pairs.
	int pair_num;
	// The mapping from color pixels to color pairs, all items initialized.
	char pixel2pair[64];
};

// Initialize an empty color map.
void color_map_init(struct color_map *map);

// Initialize a pair for the pixel. The return value is the color pair number.
// Pair 0 will be given if the pixel could not be mapped to a color pair.
int color_map_add_pair(struct color_map *map, d3d_pixel pix);

// Register all color pairs with Curses.
int color_map_apply(struct color_map *map);

// Count the number of successfully initialized pairs.
size_t color_map_count_pairs(struct color_map *map);

// Get the color pair number corresponding to a pixel.
int color_map_get_pair(struct color_map *map, d3d_pixel pix);

// Free resources associated with the map.
void color_map_destroy(struct color_map *map);

// A representation of part of the screen.
struct screen_area {
	int x, y;
	int width, height;
};

// Configuration for drawing an analog meter on the screen.
struct meter {
	// The text to label the meter with.
	const char *label;
	// Meter fullness out of 1.
	double fraction;
	// The curses style of the meter where it is full and where it is empty.
	chtype full_style, empty_style;
	// The x and y position of the meter, in character cells.
	int x, y;
	// The width of the meter, in character cells.
	int width;
	// The window to draw to.
	WINDOW *win;
};

// Draw the meter.
void meter_draw(const struct meter *meter);

// Create a small window in the middle of the screen. The window will contain
// the given text with each line centered.
WINDOW *popup_window(const char *text);

// Copy the current scene from the camera to given area. The color map is used
// to translate pixels for Curses.
void display_frame(d3d_camera *cam, struct screen_area *area,
	struct color_map *colors);

// Create a camera with the given positive dimensions.
d3d_camera *camera_with_dims(int width, int height);

// Sets the user-visible application title if possible.
void set_application_title(const char *title);

// If the terminal is known to have resized, update LINES and COLS if needed.
void update_term_size(void);

#endif /* UI_UTIL_H_ */
