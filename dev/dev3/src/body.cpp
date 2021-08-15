#include "body.h"
#include <tgmath.h>

bool bodies_collide(struct body *a, struct body *b)
{
	d3d_scalar touch_dist = a->radius + b->radius;
	if (fabs(a->pos.x - b->pos.x) < touch_dist
	 && fabs(a->pos.y - b->pos.y) < touch_dist) {
		a->health -= b->damage;
		b->health -= a->damage;
		return true;
	}
	return false;
}
