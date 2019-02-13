#ifndef __COORD_H_
#define __COORD_H_

#define MAX_ZOOM_LEVEL 18
#define MIN_ZOOM_LEVEL 4

struct PointInt
{
	int x, y;
};

struct PointFloat
{
	float x, y;
};

struct RectFloat
{
	float left, top, right, bottom;
};

bool PointInRect(const RectFloat* r, float x, float y);

static inline bool PointFloatEq(const PointFloat* a, const PointFloat* b)
{
	return (a->x == b->x) && (a->y == b->y);
}

float lon2tilex(float lon, unsigned int z);
float lat2tiley(float lat, unsigned int z);

#endif
