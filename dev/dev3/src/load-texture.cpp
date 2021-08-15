#include "load-texture.h"
#include "loader.h"
#include "logger.h"
#include "read-lines.h"
#include "pixel.h"
#include "string.h"
#include "util.h"
#include "xalloc.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static d3d_texture *new_empty_texture(void)
{
	return assert_alloc(d3d_new_texture(1, 1, TRANSPARENT_PIXEL));
}

d3d_texture *load_texture(struct loader *ldr, const char *name)
{
	FILE *file;
	d3d_texture *txtr = NULL, **txtrp;
	txtrp = loader_texture(ldr, name, &file);
	if (!txtrp) return NULL;
	txtr = *txtrp;
	if (txtr) return txtr;
	size_t width;
	size_t height;
	struct string *lines = read_lines(file, &height);
	if (!lines) {
		logger_printf(loader_logger(ldr), LOGGER_ERROR,
			"Error while reading lines: %s\n", strerror(errno));
		return NULL;
	}
	width = 0;
	for (size_t y = 0; y < height; ++y) {
		if (lines[y].len > width) width = lines[y].len;
	}
	struct color_map *colors = loader_color_map(ldr);
	if (width > 0 && height > 0) {
		txtr = assert_alloc(d3d_new_texture(width, height,
			TRANSPARENT_PIXEL));
		for (size_t y = 0; y < height; ++y) {
			struct string *line = &lines[y];
			size_t x;
			for (x = 0; x < line->len; ++x) {
				d3d_pixel pix = pixel_from_char(line->text[x]);
				int pair = color_map_add_pair(colors, pix);
				if (pair <= 0 && pix != TRANSPARENT_PIXEL)
					logger_printf(loader_logger(ldr),
						LOGGER_WARNING,
						"Pixel not registered: %c\n",
						line->text[x]);
				*d3d_texture_get(txtr, x, y) = pix;
			}
		}
	} else {
		txtr = new_empty_texture();
	}
	for (size_t i = 0; i < height; ++i) {
		free(lines[i].text);
	}
	free(lines);
	*txtrp = txtr;
	return txtr;
}
