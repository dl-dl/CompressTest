#pragma once

#include "types.h"
#include "sizes.h"
#include "coord.h"

static const ui32 NUM_IMS_BLOCKS = 1000;

struct ImsIndexDescr
{
	ui32 nx, ny; // number of tiles
	ui32 left, top; // world tile numbers
	BlockAddr firstBlock;
	// firstBlock - address of the first sector of an array of TileDescr.
	// This array contains addresses of all tiles for the zoom level.
	// The array is sorted by top, left. ([x0, y0] [x1, y0] [x2, y0], [x0, y1] [x1, y1] ...
};
#pragma pack(push, 1)
struct IMS
{
	ui32 version;
	ui32 status;
	RectInt coord;
	ui16 name[32];
	BlockAddr dataHWM;
	BlockAddr indexHWM;
	ImsIndexDescr index[MAX_ZOOM_LEVEL - MIN_ZOOM_LEVEL + 1]; // zoom levels
	ui32 checksum;
};

enum ImsStatus
{
	IMS_EMPTY = 0, IMS_READY = 1
};

struct TileIndexItem
{
	BlockAddr addr;
	ui32 sz;
};

static const ui32 INDEX_ITEMS_PER_BLOCK = (BLOCK_SIZE - sizeof(ui32)) / sizeof(TileIndexItem);

struct TileIndexBlock
{
	TileIndexItem idx[INDEX_ITEMS_PER_BLOCK];
	ui32 checksum;
};
#pragma pack(pop)

struct NewMapStatus
{
	BlockAddr imsAddr;
	ui8 currentZoom;
	ui32 tilesAtCurrentZoom;
	TileIndexBlock currentIndexBlock;
};

void FsFormat(int id);
BlockAddr FsFreeSpace(int id);
bool FsNewIMS(IMS* ims, BlockAddr* addr, const RectInt* coord, int id);
bool FsFindIMS(int x, int y, IMS *dst, int id);
void FsCommitIMS(IMS* ims, BlockAddr addr, int id);
TileIndexItem FsFindTile(const IMS* ims, ui8 zoom, ui32 numx, ui32 numy, int id);
void FsReadTile(BlockAddr addr, ui32 sz, ui8* dst, int id);

void ImsNextZoom(IMS* ims, NewMapStatus* status, ui8 zoom);
bool ImsAddTile(IMS* ims, NewMapStatus* status, const ui8* tile, ui32 sz, int id);
