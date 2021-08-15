#include "loader.h"
#include "ent.h"
#include "map.h"
#include "logger.h"
#include "pixel.h"
#include "string.h"
#include "util.h"
#include "xalloc.h"
#include <stdlib.h>
#include <string.h>

void loader_init(struct loader *ldr, const char *root)
{
	ldr->txtrs_dir = mid_cat(root, DIRSEP, "textures");
	table_init(&ldr->txtrs, 16);
	ldr->ents_dir = mid_cat(root, DIRSEP, "ents");
	table_init(&ldr->ents, 16);
	ldr->maps_dir = mid_cat(root, DIRSEP, "maps");
	table_init(&ldr->maps, 16);
	color_map_init(&ldr->colors);
	// Default background pixel:
	color_map_add_pair(&ldr->colors, pixel(PC_BLACK, PC_BLACK));
	ldr->log = NULL;
	ldr->empty_txtr = NULL;
}

char *loader_map_path(const struct loader *ldr, const char *name)
{
	size_t cap = 10;
	struct string buf;
	string_init(&buf, cap);
	string_pushz(&buf, &cap, ldr->maps_dir);
	string_pushc(&buf, &cap, DIRSEP);
	string_pushz(&buf, &cap, name);
	string_pushn(&buf, &cap, ".json", 6);
	string_shrink_to_fit(&buf);
	return buf.text;
}

static void **load(table *tab, const char *root, const char *name, FILE **file,
	struct logger *log)
{
	void **item = table_get(tab, name);
	*file = NULL;
	if (!item) {
		char *path = mid_cat(root, DIRSEP, name);
		*file = fopen(path, "r");
		if (*file) {
			logger_printf(log, LOGGER_INFO, "Loading %s\n", path);
			table_add(tab, name, NULL);
			item = table_get(tab, name);
		} else {
			logger_printf(log, LOGGER_ERROR,
				"Cannot load %s\n", path);
		}
		free(path);
	}
	return item;
}

// Load with .json suffix
static void **loadj(table *tab, const char *root, const char *name, FILE **file,
	struct logger *log)
{
	char *fname = mid_cat(name, '.', "json");
	void **loaded = load(tab, root, fname, file, log);
	if (!*file) free(fname);
	return loaded;
}

struct ent_type **loader_ent(struct loader *ldr, const char *name, FILE **file)
{
	return (struct ent_type **)loadj(&ldr->ents, ldr->ents_dir, name, file,
		ldr->log);
}

struct map **loader_map(struct loader *ldr, const char *name, FILE **file)
{
	return (struct map **)loadj(&ldr->maps, ldr->maps_dir, name, file,
		ldr->log);
}

d3d_texture **loader_texture(struct loader *ldr, const char *name, FILE **file)
{
	char *fname = str_dup(name);
	d3d_texture **loaded = (d3d_texture **)load(&ldr->txtrs, ldr->txtrs_dir,
		fname, file, ldr->log);
	if (!*file) free(fname);
	return loaded;
}

const d3d_texture *loader_empty_texture(struct loader *ldr)
{
	if (!ldr->empty_txtr)
		ldr->empty_txtr = assert_alloc(d3d_new_texture(1, 1,
			TRANSPARENT_PIXEL));
	return ldr->empty_txtr;
}

void loader_print_summary(struct loader *ldr)
{
	logger_printf(ldr->log, LOGGER_INFO,
		"Load summary: %lu maps, %lu entity types, %lu textures, "
		"%lu color pairs\n",
		(unsigned long)table_count(&ldr->maps),
		(unsigned long)table_count(&ldr->ents),
		(unsigned long)table_count(&ldr->txtrs),
		(unsigned long)color_map_count_pairs(&ldr->colors));
}

struct color_map *loader_color_map(struct loader *ldr)
{
	return &ldr->colors;
}

struct logger *loader_logger(struct loader *ldr)
{
	return ldr->log;
}

struct logger *loader_set_logger(struct loader *ldr, struct logger *log)
{
	struct logger *old = ldr->log;
	ldr->log = log;
	return old;
}

void loader_free(struct loader *ldr)
{
	const char *key;
	void **val;
	d3d_free_texture(ldr->empty_txtr);
	TABLE_FOR_EACH(&ldr->txtrs, key, val) {
		free((char *)key);
		d3d_free_texture(*val);
	}
	table_free(&ldr->txtrs);
	free(ldr->txtrs_dir);
	TABLE_FOR_EACH(&ldr->ents, key, val) {
		free((char *)key);
		ent_type_free(*val);
	}
	table_free(&ldr->ents);
	free(ldr->ents_dir);
	TABLE_FOR_EACH(&ldr->maps, key, val) {
		free((char *)key);
		map_free(*val);
	}
	table_free(&ldr->maps);
	free(ldr->maps_dir);
	color_map_destroy(&ldr->colors);
}

#if CTF_TESTS_ENABLED

#	include "libctf.h"
#	include <assert.h>

CTF_TEST(loader_loads_only_once,
	FILE *file;
	struct loader ldr;
	loader_init(&ldr, "data");
	d3d_texture *empty = assert_alloc(d3d_new_texture(0, 0,
		TRANSPARENT_PIXEL));
	d3d_texture **loaded = loader_texture(&ldr, "empty", &file);
	*loaded = empty;
	assert(*loader_texture(&ldr, "empty", &file) == empty);
	loader_free(&ldr);
)

CTF_TEST(loader_nonexistent_gives_null,
	FILE *file;
	struct loader ldr;
	loader_init(&ldr, "data");
	assert(!loader_map(&ldr, "doesn't exist", &file));
	loader_free(&ldr);
)

#endif /* CTF_TESTS_ENABLED */
