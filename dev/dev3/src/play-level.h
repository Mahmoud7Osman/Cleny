#ifndef PLAY_LEVEL_H_
#define PLAY_LEVEL_H_

// Weak dependencies
struct save_state;
struct ticker;
struct logger;

// Play a level until death, completion, or quitting. root_dir is the root game
// data directory path. save is the save being used; it will be updated if the
// player wins. map_name is the name of the map to load. timer is the timepiece
// to measure by. log is the logger to print to. If the map is nonexistent or
// locked in the given save, -1 is returned, otherwise 0.
int play_level(const char *root_dir, struct save_state *save,
	const char *map_name, struct ticker *timer, struct logger *log);

#endif /* PLAY_LEVEL_H_ */
