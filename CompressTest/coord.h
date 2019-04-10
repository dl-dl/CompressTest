#ifndef __COORD_H_
#define __COORD_H_
#include "types.h"

#define MAX_ZOOM_LEVEL 18
#define MIN_ZOOM_LEVEL 4

typedef struct
{
 int x, y;
} PointInt;

typedef struct
{
 int left, top, right, bottom;
} RectInt;

typedef struct
{
 float x, y;
} PointFloat;

typedef struct
{
 float left, top, right, bottom;
} RectFloat;

static inline bool PointInRectInt(const RectInt *r, int x, int y)
{
 if (x >= r->left && x <= r->right)
  if (y >= r->top && y <= r->bottom)
   return true;
 return false;
}

static inline bool PointEqInt(const PointInt *a, const PointInt *b)
{
 return (a->x == b->x) && (a->y == b->y);
}

static inline ui32 ScaleDownCoord(ui32 c, ui8 zoom)
{
 return c >> (MAX_ZOOM_LEVEL - zoom);
}

static inline ui32 ScaleUpCoord(ui32 c, ui8 zoom)
{
 return c << (MAX_ZOOM_LEVEL - zoom);
}

#ifdef __cplusplus
extern "C"
{
#endif
 ui32 lon2tilex(float lon, unsigned int zoom);
 ui32 lat2tiley(float lat, unsigned int zoom);
#ifdef __cplusplus
}
#endif
#endif
