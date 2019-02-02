#include "coord.h"
#include <math.h>
static const float F_PI = 3.14159265358979323846f;

bool PointInRect(const RectFloat* r, float x, float y)
{
	if (x >= r->left && x <= r->right)
		if (y <= r->top && y >= r->bottom)
			return true;
	return false;
}

float lon2tilex(float x, unsigned int z)
{
	return (x + 180) / 360 * (1 << z);
}

float lat2tiley(float y, unsigned int z)
{
	return (1 - logf(tanf(y * F_PI / 180) + 1 / cosf(y * F_PI / 180)) / F_PI) * (1 << (z - 1));
}
