#include "fs.h"
#include "fs.h"
#include "fscache.h"
#include "graph.h"
#include "sizes.h"
#include "color.h"

extern ui32 CoordTileX;
extern ui32 CoordTileY;
extern ui8 MapZoom;

typedef struct
{
 FsMapCache FsCache;
 PointInt screenStart;
} MapData;

static MapData map;

void MapInit(void)
{
 CacheInit(&map.FsCache);
 map.screenStart.x = map.screenStart.y = -1;
}

static inline int intabs(int a)
{
 return (a >= 0) ? a : -a;
}

static PointInt AdjustScreenPos(ui32 x, ui32 y, const PointInt *start)
{
 PointInt pos;
 pos.x = ScaleDownCoord(x, MapZoom);
 pos.y = ScaleDownCoord(y, MapZoom);
 pos.x -= SCREEN_DX / 2;
 pos.y -= SCREEN_DY / 2;

 if ((intabs(start->x - pos.x) > SCREEN_DX / 8) ||
     (intabs(start->y - pos.y) > SCREEN_DY / 8))
  {
   return pos;
  }
 else
  {
   return *start;
  }
}

void DrawMap()
{
 if (!CacheFetchIMS(&map.FsCache, CoordTileX / TILE_DX, CoordTileY / TILE_DY))
  return;

 map.screenStart = AdjustScreenPos(CoordTileX, CoordTileY, &map.screenStart);
 for (int x = (map.screenStart.x / TILE_DX) * TILE_DX; x < map.screenStart.x + SCREEN_DX; x += TILE_DX)
  for (int y = (map.screenStart.y / TILE_DY) * TILE_DY; y < map.screenStart.y + SCREEN_DY; y += TILE_DY)
   {
    ui32 index = CacheRead(&map.FsCache, x / TILE_DX, y / TILE_DY, MapZoom);
    CopyTileToScreen(map.FsCache.map[index].data, x - map.screenStart.x, (y - map.screenStart.y) / 2);
   }
}

static void DrawTouristMarker(ui32 x, ui32 y, int dd, int hardwareId)
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
 /*
 for (ui32 i = 0; i < dev.group.n; ++i)
  {
   const GroupItem *p = &dev.group.g[i];
   ui32 x = ScaleDownCoord(p->x, dev.zoom) - dev.screenStart.x;
   ui32 y = ScaleDownCoord(p->y, dev.zoom) - dev.screenStart.y;
   DrawTouristMarker(x, SCREEN_DY - y, 5, p->hardwareId);
  }
*/
 ui32 x = ScaleDownCoord(CoordTileX, MapZoom) - map.screenStart.x;
 ui32 y = ScaleDownCoord(CoordTileY, MapZoom) - map.screenStart.y;
 DrawTouristMarker(x, SCREEN_DY - y, 10, 1);
}
