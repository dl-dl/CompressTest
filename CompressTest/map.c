#include "graph.h"
#include "fs.h"
#include "fscache.h"
#include "sizes.h"
#include "color.h"
#include "tourist.h"

extern ui32 CoordTileX;
extern ui32 CoordTileY;
extern si8 MapShiftH;
extern si8 MapShiftV;
extern ui8 MapZoom;
extern TTourist Tourist[10];

typedef struct
{
 FsMapCache FsCache;
 PointInt screenCenter;
} MapData;

static MapData map;

void MapInit(void)
{
 CacheInit(&map.FsCache);
}

static inline int intabs(int a)
{
 return (a >= 0) ? a : -a;
}

static PointInt AdjustScreenPos(ui32 x, ui32 y, PointInt center)
{
 static ui8 mode;
 static PointInt prevShift;
 static PointInt ref;

 if (mode)
  {
   if (0 == MapShiftH && 0 == MapShiftV)
    mode = 0;
  }
 else
  {
   if (MapShiftH || MapShiftV)
    {
     mode = 1;
     ref = center;
     prevShift.x = prevShift.y = 0;
    }
  }

 if (mode)
  {
   ref.x += (MapShiftH - prevShift.x) * ScaleUpCoord(SCREEN_DX / 2, MapZoom);
   ref.y += (MapShiftV - prevShift.y) * ScaleUpCoord(SCREEN_DY / 2, MapZoom);
   prevShift.x = MapShiftH;
   prevShift.y = MapShiftV;
   return ref;
  }

 PointInt pos;
 pos.x = x;
 pos.y = y;

 if (((ui32)intabs(center.x - pos.x) > ScaleUpCoord(SCREEN_DX / 8, MapZoom)) ||
     ((ui32)intabs(center.y - pos.y) > ScaleUpCoord(SCREEN_DY / 8, MapZoom)))
  {
   return pos;
  }
 else
  {
   return center;
  }
}

void DrawMap()
{
 if (!CacheFetchIMS(&map.FsCache, CoordTileX / TILE_DX, CoordTileY / TILE_DY))
  return;

 map.screenCenter = AdjustScreenPos(CoordTileX, CoordTileY, map.screenCenter);
 PointInt start;
 start.x = ScaleDownCoord(map.screenCenter.x, MapZoom) - SCREEN_DX / 2;
 start.y = ScaleDownCoord(map.screenCenter.y, MapZoom) - SCREEN_DY / 2;

 for (int x = (start.x / TILE_DX) * TILE_DX; x < start.x + SCREEN_DX; x += TILE_DX)
  for (int y = (start.y / TILE_DY) * TILE_DY; y < start.y + SCREEN_DY; y += TILE_DY)
   {
    ui32 index = CacheRead(&map.FsCache, x / TILE_DX, y / TILE_DY, MapZoom);
    CopyTileToScreen(map.FsCache.map[index].data, x - start.x, (y - start.y) / 2);
   }
}

static void DrawTouristMarker(int x, int y, int dd, int hardwareId)
{
 for (int k = -1; k <= 1; ++k)
  DisplayLine(x - dd, y + k, x + dd, y + k, (k == 0) ? CLR_RED : CLR_WHITE);
 for (int k = -1; k <= 1; ++k)
  DisplayLine(x + k, y - dd, x + k, y + dd, (k == 0) ? CLR_RED : CLR_WHITE);
 char s[2];
 s[0] = '0' + hardwareId % 8;
 s[1] = 0;
 DisplayText(s, x + 2, y, 0, CLR_RED);
}

void DrawGroup()
{
 for (ui32 i = 0; i < sizeof(Tourist) / sizeof(*Tourist); ++i)
  if (Tourist[i].IsRx)
   {
    const TTourist *p = Tourist + i;
    int x = ScaleDownCoord(p->CoordTileX, MapZoom) - ScaleDownCoord(map.screenCenter.x, MapZoom) + SCREEN_DX / 2;
    int y = ScaleDownCoord(p->CoordTileY, MapZoom) - ScaleDownCoord(map.screenCenter.y, MapZoom) + SCREEN_DY / 2;
    DrawTouristMarker(x, SCREEN_DY - y, 5, i);
   }

 int x = ScaleDownCoord(CoordTileX, MapZoom) - ScaleDownCoord(map.screenCenter.x, MapZoom) + SCREEN_DX / 2;
 int y = ScaleDownCoord(CoordTileY, MapZoom) - ScaleDownCoord(map.screenCenter.y, MapZoom) + SCREEN_DY / 2;
 DrawTouristMarker(x, SCREEN_DY - y, 10, 1);
}
