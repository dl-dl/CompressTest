#include "coord.h"
#include <math.h>
static const float F_PI = 3.14159265358979323846f;

ui32 lon2tilex(float x, unsigned int z)
{
	return (ui32)((x + 180) / 360 * (1 << (z + 8))); // 8 for TILE_CX == 256
}

ui32 lat2tiley(float y, unsigned int z)
{
	return (ui32)((1 - logf(tanf(y * F_PI / 180) + 1 / cosf(y * F_PI / 180)) / F_PI) * (1 << (z - 1 + 8))); // 8 for TILE_CY == 256
}
