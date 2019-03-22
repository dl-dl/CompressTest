#ifndef __FSCHACHE_H
#define __FSCHACHE_H
#include "types.h"
#include "sizes.h"
#include "fs.h"

typedef struct
{
 ui8 zoom;
 ui32 tileX, tileY;
 ui8 data[TILE_CX * TILE_CY / 2];
} MapCacheItem;

typedef struct
{
 IMS ims;
 MapCacheItem map[6];
} FsMapCache;

#ifdef __cplusplus
extern "C"
{
#endif
 void CacheInit(FsMapCache *cache);
 bool CacheFetchIMS(FsMapCache *cache, ui32 x, ui32 y);
 ui32 CacheRead(FsMapCache *cache, ui32 tileX, ui32 tileY, ui32 zoom);
#ifdef __cplusplus
}
#endif
#endif // !__FSCHACHE_H
