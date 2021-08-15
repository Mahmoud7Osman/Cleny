#ifndef BODY_H_
#define BODY_H_

#include "d3d.h"
#include <stdbool.h>

// A physical representation of an entity.
struct body {
	// The entity position.
	d3d_vec_s pos;
	// The girth of the entity.
	d3d_scalar radius;
	// The remaining health of the entity.
	double health;
	// The damage done by the entity.
	double damage;
};

// Collide two bodies. If they are touching, each will damage the other.
// Returned is whether a collision did occur.
bool bodies_collide(struct body *a, struct body *b);

#endif /* BODY_H_ */
