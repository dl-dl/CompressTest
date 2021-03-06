#ifndef _FS_H_
#define _FS_H_

#include "types.h"
#include "coord.h"

#define CUR_MAP_FILE_VERSION 2

#pragma pack(push, 1)
typedef struct
{
 ui32 nx, ny;    // number of tiles
 ui32 left, top; // world tile numbers
 FileAddr firstBlock;
 // firstBlock - address of an array that contains addresses of all tiles for the zoom level.
 // The array is sorted by top, left. ([x0, y0] [x1, y0] [x2, y0], [x0, y1] [x1, y1] ...
} ImsIndexDescr;

typedef struct
{
 ui32 version;
 ui8 zoomMin, zoomMax;
 ui16 reserved;
 RectInt coord;
 ImsIndexDescr index[MAX_ZOOM_LEVEL - MIN_ZOOM_LEVEL + 1]; // zoom levels
 ui32 checksum;
} IMS;

typedef struct
{
 FileAddr addr;
 FileAddr next;
} TileIndexItem;

#pragma pack(pop)

typedef struct
{
 IMS ims;
 bool dataReady;
} ExtIMS;

typedef struct
{
 ui8 currentZoom;
 ui32 tilesAtCurrentZoom;
 FileAddr dataHWM;
} NewMapStatus;

#ifdef __cplusplus
extern "C"
{
#endif
 ui32 MapCalcCRC(const void *data, ui32 sz);
 void MapFindIMS(int x, int y, ExtIMS *dst);
 TileIndexItem MapFindTile(const IMS *ims, ui8 zoom, ui32 numx, ui32 numy);
 void MapReadTile(FileAddr addr, ui32 sz, ui8 *dst);
#ifdef __cplusplus
}
#endif
#endif
