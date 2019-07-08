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
 memset(&cache->ims, 0, sizeof(cache->ims));
 cache->fnum = 1;
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
 if (cache->fnum && PointInRectInt(&cache->ims.coord, x, y))
  return;

 cache->fnum = MapFindIMS(x, y, &cache->ims);
}

ui32 CacheRead(MapTileCache *cache, ui32 tileX, ui32 tileY, ui32 zoom)
{
 char name[32];
 name[0] = 'f';
 name[1] = '0' + cache->fnum;
 strcpy(name + 2, ".map");
 file_open(name, false);
 ui32 index = CacheIndex(tileX, tileY);
 if (!CacheEq(&cache->map[index], tileX, tileY, zoom))
  {
   TileIndexItem i = MapFindTile(&cache->ims, zoom, tileX, tileY);
   if (i.addr)
    MapReadTile(i.addr, i.sz, cache->map[index].data);
   else
    memset(cache->map[index].data, 0x55, sizeof(cache->map[0].data));
   cache->map[index].zoom = zoom;
   cache->map[index].tileX = tileX;
   cache->map[index].tileY = tileY;
  }
 file_close();
 return index;
}
