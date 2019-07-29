#include "graph.h"
#include "fs.h"
#include "fscache.h"
#include "map.h"

extern ui32 CoordTileX;
extern ui32 CoordTileY;
extern si8 MapShiftH;
extern si8 MapShiftV;
extern si8 MapZoom;

static MapTileCache FsCache;
static PointInt screenCenter;

void MapInit(void)
{
 CacheInit(&FsCache);
}

void MapDeinit(void)
{
 CacheDeinit(&FsCache);
}

static inline int intabs(int a)
{
 return (a >= 0) ? a : -a;
}

void AdjustScreenPos(ui32 x, ui32 y)
{
 static ui8 mode;
 static PointInt prevShift;
 static PointInt ref;

 if (mode)
  {
   if ((0 == MapShiftH) && (0 == MapShiftV))
    mode = 0;
  }
 else
  {
   if (MapShiftH || MapShiftV)
    {
     mode = 1;
     ref = screenCenter;
     prevShift.x = prevShift.y = 0;
    }
  }

 if (mode)
  {
   ref.x += (MapShiftH - prevShift.x) * ScaleUpCoord(SCREEN_DX / 2, MapZoom);
   ref.y += (MapShiftV - prevShift.y) * ScaleUpCoord(SCREEN_DY / 2, MapZoom);
   prevShift.x = MapShiftH;
   prevShift.y = MapShiftV;
   screenCenter = ref;
   return;
  }

 PointInt pos;
 pos.x = x;
 pos.y = y;

 if (((ui32)intabs(screenCenter.x - pos.x) > ScaleUpCoord(SCREEN_DX / 8, MapZoom)) ||
     ((ui32)intabs(screenCenter.y - pos.y) > ScaleUpCoord(SCREEN_DY / 8, MapZoom)))
  {
   screenCenter = pos;
  }
}

void AdjustZoom()
{
 if (FsCache.eims.dataReady)
  {
   if (MapZoom < FsCache.eims.ims.zoomMin)
    MapZoom = FsCache.eims.ims.zoomMin;
   else if (MapZoom > FsCache.eims.ims.zoomMax)
    MapZoom = FsCache.eims.ims.zoomMax;
  }
}

PointInt GetScreenTopLeft()
{
 PointInt p;
 p.x = ScaleDownCoord(screenCenter.x, MapZoom) - SCREEN_DX / 2;
 p.y = ScaleDownCoord(screenCenter.y, MapZoom) - SCREEN_DY / 2;
 return p;
}

PointInt GetScreenCenter()
{
 return screenCenter;
}

void DrawMap()
{
 AdjustZoom();
 AdjustScreenPos(CoordTileX, CoordTileY);

 CacheFetchIMS(&FsCache, CoordTileX / TILE_DX, CoordTileY / TILE_DY);
 if (FsCache.eims.dataReady)
  {
   PointInt start = GetScreenTopLeft();
   for (int x = (start.x / TILE_DX) * TILE_DX; x < start.x + SCREEN_DX; x += TILE_DX)
    for (int y = (start.y / TILE_DY) * TILE_DY; y < start.y + SCREEN_DY; y += TILE_DY)
     {
      ui32 index = CacheRead(&FsCache, x / TILE_DX, y / TILE_DY, MapZoom);
      CopyTileToScreen(FsCache.map[index].data, x - start.x, (y - start.y) / 2);
     }
  }
 else
  {
   DisplayClear(0);
  }
}
