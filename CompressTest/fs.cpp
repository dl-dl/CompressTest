#include "sd.h"
#include "fs.h"
#include "coord.h"
#include <string.h>
#include <assert.h>

static ui32 calcCRC(const void* data, ui32 sz) // TODO: implement proper algorithm
{
	ui32 crc = 0;
	for (ui32 i = 0; i < sz; ++i)
		crc += *((ui8*)data + i);
	return crc;
}

void fsFormat(char id)
{
	ui8 b[BLOCK_SIZE];
	memset(b, 0, sizeof(b));
	for (BlockAddr i = 0; i < NUM_IMS_BLOCKS; ++i)
		sdCardWrite(i, b, id);
}

static bool findFirstEmptyIMS(BlockAddr* dataHWM, BlockAddr* indexHWM, BlockAddr* addr, char id)
{
	ui8 b[BLOCK_SIZE];
	*indexHWM = NUM_IMS_BLOCKS;
	*dataHWM = sdCardSize() - 1;
	for (BlockAddr i = 0; i < NUM_IMS_BLOCKS; ++i)
	{
		sdCardRead(i, b, id);
		const IMS *p = (IMS *)b;
		if (IMS_EMPTY == p->status)
		{
			*addr = i;
			return true;
		}
	}
	return false;
}

bool fsAddIMS(IMS* ims, BlockAddr* addr, const RectFloat* coord, char id)
{
	BlockAddr dataHWM, indexHWM;
	if (!findFirstEmptyIMS(&dataHWM, &indexHWM, addr, id))
		return false;

	memset(ims, 0, sizeof(*ims));
	ims->status = IMS_EMPTY;
	ims->coord = *coord;
	ims->name[0] = 0;
	ims->dataHWM = dataHWM;
	ims->indexHWM = indexHWM;

	for (ui8 z = 1; z < MAX_ZOOM_LEVEL; ++z)
	{
		ims->index[z].firstBlock = 0;
		ims->index[z].left = (ui32)lon2tilex(ims->coord.left, z);
		ims->index[z].top = (ui32)lat2tiley(ims->coord.top, z);
		ui32 dx = (ui32)lon2tilex(ims->coord.right, z) - ims->index[z].left + 1;
		ui32 dy = (ui32)lat2tiley(ims->coord.bottom, z) - ims->index[z].top + 1;
		ims->index[z].nx = dx;
		ims->index[z].ny = dy;
		ui32 numBlocks = (dx * dy) / INDEX_ITEMS_PER_BLOCK;
		if ((dx * dy) % INDEX_ITEMS_PER_BLOCK)
			numBlocks++;
		ims->indexHWM += numBlocks;
	}

	return addr;
}

bool fsFindIMS(float x, float y, IMS *dst, char id)
{
	ui8 b[BLOCK_SIZE];
	for (BlockAddr i = 0; i < NUM_IMS_BLOCKS; ++i)
	{
		sdCardRead(i, b, id);
		IMS *ims = (IMS *)b;
		ui32 checksum = ims->checksum;
		ims->checksum = 0;
		if (checksum != calcCRC(ims, sizeof(*ims)))
			return false;
		if (IMS_EMPTY == ims->status)
			return false;
		if (IMS_READY == ims->status)
		{
			if (x >= ims->coord.left && x < ims->coord.right)
				if (y >= ims->coord.bottom && y < ims->coord.top)
				{
					*dst = *ims;
					return true;
				}
		}
	}
	return false;
}

static ui32 findTileColA(ui8 zoom, ui32 mapLeft, float x)
{
	return (ui32)lon2tilex(x, zoom) - mapLeft;
}

static ui32 findTileRowA(ui8 zoom, ui32 mapTop, float y)
{
	return (ui32)lat2tiley(y, zoom) - mapTop;
}

#if 0
static ui32 findTileColB(const ImsIndexDescr *zi, float x)
{
	ui8 b[BLOCK_SIZE];
	ui32 offset = 0;
	ui32 offsetOld = -1;
	for (ui32 i = 0; i < zi->nx; ++i) // linear search. can be implemented as binary search
	{
		offset = i / (BLOCK_SIZE / sizeof(TileDescr));
		if (offset != offsetOld)
		{
			SdCardRead(zi->firstBlock + offset, b);
			offsetOld = offset;
		}
		ui32 j = i % (BLOCK_SIZE / sizeof(TileDescr));
		const TileDescr *td = (TileDescr *)b;
		if (td[j].left <= x && x < td[j].right)
			return i;
	}
	return 0;
}

static ui32 findTileRowB(const ImsIndexDescr *zi, float y)
{
	ui8 b[BLOCK_SIZE];
	ui32 offset = 0;
	ui32 offsetOld = -1;
	for (ui32 i = 0; i < zi->ny; ++i) // linear search. can be implemented as binary search
	{
		offset = (i * zi->nx) / (BLOCK_SIZE / sizeof(TileDescr));
		if (offset != offsetOld)
		{
			SdCardRead(zi->firstBlock + offset, b);
			offsetOld = offset;
		}
		ui32 j = (i * zi->nx) % (BLOCK_SIZE / sizeof(TileDescr));
		const TileDescr *td = (TileDescr *)b;
		if (td[j].left <= y && y < td[j].right)
			return i;
	}
	return 0;
}
#endif

BlockAddr fsFindTile(const IMS* ims, ui8 zoom, ui32 numx, ui32 numy, char id)
{
	if (0 == ims->index[zoom].firstBlock)
		return 0;

	ui32 dx = (ui32)numx - ims->index[zoom].left;
	ui32 dy = (ui32)numy - ims->index[zoom].top;
	if (dx >= ims->index[zoom].nx || dy >= ims->index[zoom].ny)
		return 0;

	ui32 offs = dx * ims->index[zoom].ny + dy;
	ui8 b[BLOCK_SIZE];
	sdCardRead(ims->index[zoom].firstBlock + offs / INDEX_ITEMS_PER_BLOCK, b, id);
	const IndexBlock* p = (IndexBlock*)b;
	if (p->checksum != calcCRC(p->idx, sizeof(p->idx)))
		return 0;
	return p->idx[offs % INDEX_ITEMS_PER_BLOCK];
}

void imsNextZoom(IMS* ims, NewMapStatus* status, ui8 zoom)
{
	status->currentZoom = zoom;
	status->tilesAtCurrentZoom = 0;
	memset(status->currentIndexBlock.idx, 0xFD, sizeof(status->currentIndexBlock.idx));
	ims->index[status->currentZoom].firstBlock = ims->indexHWM;
}

bool imsAddTile(IMS* ims, NewMapStatus* status, const NewTile* tile, char id)
{
	assert(ims->dataHWM >= ims->indexHWM);

	status->currentIndexBlock.idx[status->tilesAtCurrentZoom % INDEX_ITEMS_PER_BLOCK] = ims->dataHWM;
	for (ui32 written = 0; tile->size > written; written += BLOCK_SIZE) // write data
	{
		sdCardWrite(ims->dataHWM, (ui8*)tile + written, id);
		ims->dataHWM--;
		if (ims->dataHWM <= ims->indexHWM)
			return false;
	}
	status->tilesAtCurrentZoom++;

	if ((status->tilesAtCurrentZoom == ims->index[status->currentZoom].nx * ims->index[status->currentZoom].ny)
		|| (status->tilesAtCurrentZoom % INDEX_ITEMS_PER_BLOCK == 0))
	{
		if (status->tilesAtCurrentZoom % INDEX_ITEMS_PER_BLOCK)
		{
			assert(ims->indexHWM == ims->index[status->currentZoom].firstBlock + status->tilesAtCurrentZoom / INDEX_ITEMS_PER_BLOCK);
		}
		else
		{
			assert(ims->indexHWM + 1 == ims->index[status->currentZoom].firstBlock + status->tilesAtCurrentZoom / INDEX_ITEMS_PER_BLOCK);
		}
		status->currentIndexBlock.checksum = calcCRC(status->currentIndexBlock.idx, sizeof(status->currentIndexBlock.idx));
		sdCardWrite(ims->indexHWM, &status->currentIndexBlock, id);
		ims->indexHWM++;
		if (ims->dataHWM <= ims->indexHWM)
			return false;
		memset(status->currentIndexBlock.idx, 0xFD, sizeof(status->currentIndexBlock.idx)); // TODO: remove debug
	}
	return true;
}

void fsCommitIMS(IMS* ims, BlockAddr addr, char id)
{
	ims->status = IMS_READY;
	ims->checksum = 0;
	ims->checksum = calcCRC(ims, sizeof(*ims));
	ui8 b[BLOCK_SIZE];
	memcpy(b, ims, sizeof(*ims));
	sdCardWrite(addr, b, id);
}

void fsReadTile(BlockAddr addr, void* dst, char id)
{
	ui8* tile = (ui8*)dst;
	sdCardRead(addr, tile, id);
	ui32 sz = ((NewTile*)tile)->size;
	for (ui32 i = 1; i < sz / BLOCK_SIZE + 1; ++i)
	{
		sdCardRead(--addr, tile + i * BLOCK_SIZE, id);
	}
}
