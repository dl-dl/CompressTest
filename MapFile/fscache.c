#define _CRT_SECURE_NO_WARNINGS
#include "fscache.h"
#include "fileio.h"
#include <string.h>

void CacheInit(MapTileCache *cache)
{
 for (int i = 0; i < sizeof(cache->map) / sizeof(cache->map[0]); ++i)
  {
   cache->map[i].zoom = 0; // empty
  }
 memset(&cache->eims, 0, sizeof(cache->eims));
}

static inline ui32 CacheIndex(ui32 x, ui32 y)
{
 return (x % 2) + (y % 3) * 2;
}

static inline bool CacheEq(const MapCacheItem *p, ui32 x, ui32 y, ui32 z)
{
 return (p->tileX == x) && (p->tileY == y) && (p->zoom == z);
}

void CacheFetchIMS(MapTileCache *cache, ui32 x, ui32 y)
{
 if (cache->eims.fname[0] && PointInRectInt(&cache->eims.ims.coord, x, y))
  return;

 MapFindIMS(x, y, &cache->eims);
}

ui32 CacheRead(MapTileCache *cache, ui32 tileX, ui32 tileY, ui32 zoom)
{
 file_open(cache->eims.fname, false);
 ui32 index = CacheIndex(tileX, tileY);
 if (!CacheEq(&cache->map[index], tileX, tileY, zoom))
  {
   TileIndexItem i = MapFindTile(&cache->eims.ims, zoom, tileX, tileY);
   if (i.addr)
    MapReadTile(i.addr, i.next - i.addr, cache->map[index].data);
   else
    memset(cache->map[index].data, 0x55, sizeof(cache->map[0].data));
   cache->map[index].zoom = zoom;
   cache->map[index].tileX = tileX;
   cache->map[index].tileY = tileY;
  }
 file_close();
 return index;
}
