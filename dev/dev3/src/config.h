#ifndef CONFIG_H_
#define CONFIG_H_

// This file contains constants that are used in various parts of the game. Some
// may require including other headers for them to work. I could make this
// configuration loadable at runtime, but that would be a lot of work. The
// control keys are currently not included since changing them requires changing
// instructions to the player and stuff as well.

// There are this many milliseconds between game frames.
#define FRAME_DELAY 30

// The title screensaver is on the map of this name.
#define TITLE_SCREEN_MAP_NAME "title"

// In the screensaver, this number scales with the speed of one cycle.
#define TITLE_SCREEN_SPEED 0.005

// In the screensaver, the camera goes out at most this far (measured in tiles.)
#define TITLE_SCREEN_RADIUS 0.5

// The number of ticks a turn lasts, triggered by a single key press. This is to
// smooth out key detection speeds for different terminals, as some wait longer
// before registering another press.
#define TURN_DURATION 5

// Colors for the in-game health meter (requires including pixel.h).
#define HEALTH_METER_FG_COLOR PC_BLACK
#define HEALTH_METER_BG_COLOR PC_GREEN

// Colors for the in-game reload meter (requires including pixel.h).
#define RELOAD_METER_FG_COLOR PC_BLACK
#define RELOAD_METER_BG_COLOR PC_RED

// The width:height ratio of each pixel. Curses seems not to have a way to
// determine this dynamically. The information is used when deciding camera
// dimensions.
#define PIXEL_ASPECT 0.5

// The field of view side-to-side, in radians, of a camera. The field is smaller
// if the screen is taller than it is wide, which is unlikely. See
// camera_with_dims in ui-util.c for details.
#define CAM_FOV_X 1.2

// The character carrying the foreground color for pixels in the 3D scene.
#define SCENE_FG_CHAR ':'

// This is a weird constant multiplied by the player's turn "chance" to convert
// it to a reasonable constant speed in radians. It's not the best.
#define PLAYER_TURN_MULTIPLIER 2.5

#endif /* CONFIG_H_ */
