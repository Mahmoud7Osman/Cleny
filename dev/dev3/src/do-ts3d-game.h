#ifndef DO_TS3D_GAME_H_
#define DO_TS3D_GAME_H_

struct logger; // Weak dependency

// Run a game of Thing Shooter 3D. This will take control of the terminal.
// data_dir is the path of the game data root directory. state_file is the path
// of the file where persistent state is kept. The return value is 0 for success
// or -1 for some kind of failure. Log messages are printed to log.
int do_ts3d_game(const char *data_dir, const char *state_file,
		struct logger *log);

#endif /* DO_TS3D_GAME_H_ */
