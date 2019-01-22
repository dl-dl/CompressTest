#pragma once

#include "types.h"
#include "sd.h"
#include "coord.h"

static const ui32 INDEX_ITEMS_PER_BLOCK = (BLOCK_SIZE - sizeof(ui32)) / sizeof(BlockAddr);

static const ui32 NUM_IMS_BLOCKS = 1000;
static const ui32 MAX_ZOOM_LEVEL = 16;

struct ImsIndexDescr
{
	ui32 nx, ny; // number of tiles
	ui32 left, top; // world tile numbers
	BlockAddr firstBlock;
	// firstBlock - addres of the first sector of an array of TileDescr.
	// This array contains addresses of all tiles for the zoom level.
	// The array is sorted by top, left. ([x0, y0] [x1, y0] [x2, y0], [x0, y1] [x1, y1] ...
};
#pragma pack(push, 1)
struct IMS
{
	ui32 version;
	ui32 status;
	RectFloat coord;
	ui16 name[32];
	BlockAddr dataHWM;
	BlockAddr indexHWM;
	ImsIndexDescr index[MAX_ZOOM_LEVEL + 1]; // zoom levels
	ui32 checksum;
};

enum ImsStatus
{
	IMS_EMPTY = 0, IMS_READY = 1
};

struct IndexBlock
{
	BlockAddr idx[INDEX_ITEMS_PER_BLOCK];
	ui32 checksum;
};

struct NewTile
{
	ui32 size;
	ui8 data[1];
};
#pragma pack(pop)

struct NewMapStatus
{
	BlockAddr imsAddr;
	ui8 currentZoom;
	ui32 tilesAtCurrentZoom;
	IndexBlock currentIndexBlock;
};

void fsFormat(char id);
bool fsAddIMS(IMS* ims, BlockAddr* addr, const RectFloat* coord, char id);
bool fsFindIMS(float x, float y, IMS *dst, char id);
void fsCommitIMS(IMS* ims, BlockAddr addr, char id);
BlockAddr fsFindTile(const IMS* ims, ui8 zoom, ui32 numx, ui32 numy, char id);
void fsReadTile(BlockAddr addr, void* dst, char id);

void imsNextZoom(IMS* ims, NewMapStatus* status, ui8 zoom);
bool imsAddTile(IMS* ims, NewMapStatus* status, const NewTile* tile, char id);
