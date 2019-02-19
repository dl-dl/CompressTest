#ifndef __FSCHACHE_H
#define __FSCHACHE_H
#include "types.h"
#include "sizes.h"
#include "fs.h"

struct MapCacheItem
{
	ui8 zoom;
	ui32 tileX, tileY;
	ui8 data[TILE_CX * TILE_CY / 2];
};

struct FsMapCache
{
	IMS ims;
	MapCacheItem map[6];
	int id; // TODO: tmp; to be removed
};

void CacheInit(FsMapCache* cache, int id);
bool CacheFetchIMS(FsMapCache* cache, ui32 x, ui32 y);
ui32 CacheRead(FsMapCache* cache, ui32 tileX, ui32 tileY, ui32 zoom);

#endif // !__FSCHACHE_H