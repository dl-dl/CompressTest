#pragma once

#include "types.h"
#include "sizes.h"
#include "coord.h"

typedef struct
{
 ui32 nx, ny;    // number of tiles
 ui32 left, top; // world tile numbers
 BlockAddr firstBlock;
 // firstBlock - address of the first sector of an array of TileDescr.
 // This array contains addresses of all tiles for the zoom level.
 // The array is sorted by top, left. ([x0, y0] [x1, y0] [x2, y0], [x0, y1] [x1, y1] ...
} ImsIndexDescr;
#pragma pack(push, 1)
typedef struct
{
 ui32 version;
 ui32 status;
 RectInt coord;
 ui16 name[32];
 BlockAddr dataHWM;
 BlockAddr indexHWM;
 ImsIndexDescr index[MAX_ZOOM_LEVEL - MIN_ZOOM_LEVEL + 1]; // zoom levels
 ui32 checksum;
} IMS;

#define IMS_EMPTY 0
#define IMS_READY 1

typedef struct
{
 BlockAddr addr;
 ui32 sz;
} TileIndexItem;

#define INDEX_ITEMS_PER_BLOCK ((BLOCK_SIZE - sizeof(ui32)) / sizeof(TileIndexItem))

typedef struct
{
 TileIndexItem idx[INDEX_ITEMS_PER_BLOCK];
 ui32 checksum;
} TileIndexBlock;
#pragma pack(pop)

typedef struct
{
 BlockAddr imsAddr;
 ui8 currentZoom;
 ui32 tilesAtCurrentZoom;
 TileIndexBlock currentIndexBlock;
} NewMapStatus;

#ifdef __cplusplus
extern "C"
{
#endif
 void FsFormat(void);
 BlockAddr FsFreeSpace(void);
 bool FsNewIMS(IMS *ims, BlockAddr *addr, const RectInt *coord);
 bool FsFindIMS(int x, int y, IMS *dst);
 bool FsCommitIMS(IMS *ims, BlockAddr addr);
 TileIndexItem FsFindTile(const IMS *ims, ui8 zoom, ui32 numx, ui32 numy);
 void FsReadTile(BlockAddr addr, ui32 sz, ui8 *dst);

 void ImsNextZoom(IMS *ims, NewMapStatus *status, ui8 zoom);
 bool ImsAddTile(IMS *ims, NewMapStatus *status, const ui8 *tile, ui32 sz);
#ifdef __cplusplus
}
#endif
