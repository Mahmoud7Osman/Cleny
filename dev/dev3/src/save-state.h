#ifndef SAVE_STATE_H_
#define SAVE_STATE_H_

#include "table.h"
#include <stdio.h>
#include <stdbool.h>

struct logger; // Weak dependency

// A representation of a named level progress record created by the user.
struct save_state {
	table complete;
};

// Load a save from a file. The file is closed. Error messages are sent to the
// log if an error occurs, and -1 is returned (0 is for success.)
int save_state_init(struct save_state *save, FILE *from, struct logger *log);

// Check whether a named level is recorded as complete in the save.
bool save_state_is_complete(const struct save_state *save, const char *name);

// Mark a level name as complete.
void save_state_mark_complete(struct save_state *save, const char *name);

// Write a save state to a file. The file is NOT closed. -1 is returned if an
// error occurred, and the state is incompletely written.
int save_state_write(struct save_state *save, FILE *to);

// Deallocate a save state.
void save_state_destroy(struct save_state *save);

#endif /* SAVE_STATE_H_ */
