#ifndef __COORD_H_
#define __COORD_H_
#include "types.h"

#define MAX_ZOOM_LEVEL 18
#define MIN_ZOOM_LEVEL 4

struct PointInt
{
	int x, y;
};

struct RectInt
{
	int left, top, right, bottom;
};

struct PointFloat
{
	float x, y;
};

struct RectFloat
{
	float left, top, right, bottom;
};

static inline bool PointInRectInt(const RectInt* r, int x, int y)
{
	if (x >= r->left && x <= r->right)
		if (y >= r->top && y <= r->bottom)
			return true;
	return false;
}

static inline bool PointEqInt(const PointInt* a, const PointInt* b)
{
	return (a->x == b->x) && (a->y == b->y);
}

static inline ui32 ScaleDownCoord(ui32 c, ui8 zoom)
{
	return c >> (MAX_ZOOM_LEVEL - zoom);
}

ui32 lon2tilex(float lon, unsigned int zoom);
ui32 lat2tiley(float lat, unsigned int zoom);

#endif
