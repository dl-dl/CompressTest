#include "fscache.h"
#include <string.h>

void CacheInit(FsMapCache *cache, int id)
{
 for (int i = 0; i < sizeof(cache->map) / sizeof(cache->map[0]); ++i)
  {
   cache->map[i].zoom = 0; // empty
  }
 memset(&cache->ims, 0, sizeof(cache->ims));
 cache->id = id;
}

static inline ui32 CacheIndex(ui32 x, ui32 y)
{
 return (x % 2) + (y % 3) * 2;
}

static inline bool CacheEq(const MapCacheItem *p, ui32 x, ui32 y, ui32 z)
{
 return (p->tileX == x) && (p->tileY == y) && (p->zoom == z);
}

bool CacheFetchIMS(FsMapCache *cache, ui32 x, ui32 y)
{
 if (!PointInRectInt(&cache->ims.coord, x, y))
  if (!FsFindIMS(x, y, &cache->ims, cache->id))
   return false;
 return true;
}

ui32 CacheRead(FsMapCache *cache, ui32 tileX, ui32 tileY, ui32 zoom)
{
 ui32 index = CacheIndex(tileX, tileY);
 if (!CacheEq(&cache->map[index], tileX, tileY, zoom))
  {
   TileIndexItem i = FsFindTile(&cache->ims, zoom, tileX, tileY, cache->id);
   if (i.addr)
    FsReadTile(i.addr, i.sz, cache->map[index].data, cache->id);
   else
    memset(cache->map[index].data, 0x55, sizeof(cache->map[0].data));
   cache->map[index].zoom = zoom;
   cache->map[index].tileX = tileX;
   cache->map[index].tileY = tileY;
  }
 return index;
}
