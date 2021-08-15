#ifndef LOADER_H_
#define LOADER_H_

#include "d3d.h"
#include "table.h"
#include "ui-util.h"
#include <stdio.h>

// Weak dependencies
struct logger;
struct map;
struct ent_type;

// An object for loading game resources recursively. The fields are private.
struct loader {
	d3d_texture *empty_txtr;
	table txtrs;
	char *txtrs_dir;
	table ents;
	char *ents_dir;
	table maps;
	char *maps_dir;
	struct color_map colors;
	struct logger *log;
};

// Initialize a loader with the given data root directory. If the directory is
// invalid, you are currently told later when you try to load an item.
void loader_init(struct loader *ldr, const char *root);

// The following three functions load a named item. If the name is invalid or
// there was an error (in errno), NULL is returned. If the item was already
// loaded, a double pointer to it is returned. If the item can be loaded, file
// is set to the file from which it can be loaded and a pointer to be later set
// by the user to the parsed item is returned.

struct ent_type **loader_ent(struct loader *ldr, const char *name, FILE **file);

struct map **loader_map(struct loader *ldr, const char *name, FILE **file);

d3d_texture **loader_texture(struct loader *ldr, const char *name, FILE **file);

// Allocate a NUL-terminated string that is where the loader would search for
// the map with the given name. This file may or may not exist.
char *loader_map_path(const struct loader *ldr, const char *name);

// Load an empty (transparent) texture shared per loader.
const d3d_texture *loader_empty_texture(struct loader *ldr);

// Print to the INFO log a count of all that has been loaded thus far.
void loader_print_summary(struct loader *ldr);

// Get a non-owned reference to the loader's current color map.
struct color_map *loader_color_map(struct loader *ldr);

// Get a pointer to a loader's logger. Initially, this will have the default
// settings.
struct logger *loader_logger(struct loader *ldr);

// Set the logger to use, returning the old logger. The new logger must survive
// at least as long as the loader, and will not be freed by it.
struct logger *loader_set_logger(struct loader *ldr, struct logger *log);

// Free a loader and the items it has loaded (not the logger.)
void loader_free(struct loader *ldr);

#endif /* LOADER_H_ */
