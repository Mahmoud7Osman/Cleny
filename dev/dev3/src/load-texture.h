#ifndef LOAD_TEXTURE_H_
#define LOAD_TEXTURE_H_

#include "d3d.h"

struct loader; // Weak dependency

// Load a texture named as given or give one already loaded.
d3d_texture *load_texture(struct loader *ldr, const char *name);

#endif /* LOAD_TEXTURE_H_ */
