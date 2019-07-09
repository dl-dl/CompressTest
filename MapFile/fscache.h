#ifndef __FSCHACHE_H
#define __FSCHACHE_H
#include "types.h"
#include "sizes.h"
#include "fs.h"

typedef struct
{
 ui8 zoom;
 ui32 tileX, tileY;
 ui8 data[TILE_DX * TILE_DY / 2];
} MapCacheItem;

typedef struct
{
 ExtIMS eims;
 MapCacheItem map[6];
} MapTileCache;

#ifdef __cplusplus
extern "C"
{
#endif
 void CacheInit(MapTileCache *cache);
 void CacheFetchIMS(MapTileCache *cache, ui32 x, ui32 y);
 ui32 CacheRead(MapTileCache *cache, ui32 tileX, ui32 tileY, ui32 zoom);
#ifdef __cplusplus
}
#endif
#endif // !__FSCHACHE_H
