#include "sd.h"
#include "fs.h"
#include "coord.h"
#include "convert.h"
#include <string.h>
#include <assert.h>

static ui32 CalcCRC(const void* data, ui32 sz) // TODO: implement proper algorithm
{
	ui32 crc = 0;
	for (ui32 i = 0; i < sz; ++i)
		crc += *((ui8*)data + i);
	return crc;
}

void FsFormat(int id)
{
	ui8 b[BLOCK_SIZE];
	memset(b, 0, sizeof(b));
	for (BlockAddr i = 0; i < NUM_IMS_BLOCKS; ++i)
		sdCardWrite(i, b, id);
}

static bool FindFirstEmptyIMS(BlockAddr* dataHWM, BlockAddr* indexHWM, BlockAddr* addr, int id)
{
	ui8 b[BLOCK_SIZE];
	*indexHWM = NUM_IMS_BLOCKS; // init value for empty filel system
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
		*indexHWM = p->indexHWM;
		*dataHWM = p->dataHWM;
	}
	return false;
}

BlockAddr FsFreeSpace(int id)
{
	BlockAddr dataHWM, indexHWM, addr;
	if (!FindFirstEmptyIMS(&dataHWM, &indexHWM, &addr, id))
		return sdCardSize() - 2;
	return dataHWM - indexHWM;
}

bool FsNewIMS(IMS* ims, BlockAddr* addr, const RectFloat* coord, int id)
{
	BlockAddr dataHWM, indexHWM;
	if (!FindFirstEmptyIMS(&dataHWM, &indexHWM, addr, id))
		return false;

	memset(ims, 0, sizeof(*ims));
	ims->status = IMS_EMPTY;
	ims->coord = *coord;
	ims->name[0] = 0;
	ims->dataHWM = dataHWM;
	ims->indexHWM = indexHWM;

	for (ui8 z = MIN_ZOOM_LEVEL; z <= MAX_ZOOM_LEVEL; ++z)
	{
		int i = z - MIN_ZOOM_LEVEL;
		ims->index[i].firstBlock = 0;
		ims->index[i].left = (ui32)lon2tilex(ims->coord.left, z);
		ims->index[i].top = (ui32)lat2tiley(ims->coord.top, z);
		ui32 dx = (ui32)lon2tilex(ims->coord.right, z) - ims->index[i].left + 1;
		ui32 dy = (ui32)lat2tiley(ims->coord.bottom, z) - ims->index[i].top + 1;
		ims->index[i].nx = dx;
		ims->index[i].ny = dy;
		ui32 numBlocks = (dx * dy) / INDEX_ITEMS_PER_BLOCK;
		if ((dx * dy) % INDEX_ITEMS_PER_BLOCK)
			numBlocks++;
		ims->indexHWM += numBlocks;
	}

	return addr;
}

bool FsFindIMS(float x, float y, IMS *dst, int id)
{
	ui8 b[BLOCK_SIZE];
	for (BlockAddr i = 0; i < NUM_IMS_BLOCKS; ++i)
	{
		sdCardRead(i, b, id);
		IMS *ims = (IMS *)b;
		ui32 checksum = ims->checksum;
		ims->checksum = 0;
		if (checksum != CalcCRC(ims, sizeof(*ims)))
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
#if 0
static ui32 findTileColA(ui8 zoom, ui32 mapLeft, float x)
{
	return (ui32)lon2tilex(x, zoom) - mapLeft;
}

static ui32 findTileRowA(ui8 zoom, ui32 mapTop, float y)
{
	return (ui32)lat2tiley(y, zoom) - mapTop;
}

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

BlockAddr FsFindTile(const IMS* ims, ui8 zoom, ui32 numx, ui32 numy, int id)
{
	assert(zoom >= MIN_ZOOM_LEVEL);
	assert(zoom <= MAX_ZOOM_LEVEL);
	ui32 i = zoom - MIN_ZOOM_LEVEL;
	if (0 == ims->index[i].firstBlock)
		return 0;

	ui32 dx = numx - ims->index[i].left;
	ui32 dy = numy - ims->index[i].top;
	if (dx >= ims->index[i].nx || dy >= ims->index[i].ny)
		return 0;

	ui32 offs = dx * ims->index[i].ny + dy;
	ui8 b[BLOCK_SIZE];
	sdCardRead(ims->index[i].firstBlock + offs / INDEX_ITEMS_PER_BLOCK, b, id);
	const IndexBlock* p = (IndexBlock*)b;
	if (p->checksum != CalcCRC(p->idx, sizeof(p->idx)))
		return 0;
	return p->idx[offs % INDEX_ITEMS_PER_BLOCK];
}

void ImsNextZoom(IMS* ims, NewMapStatus* status, ui8 zoom)
{
	assert(zoom >= MIN_ZOOM_LEVEL);
	assert(zoom <= MAX_ZOOM_LEVEL);
	status->currentZoom = zoom;
	status->tilesAtCurrentZoom = 0;
	memset(status->currentIndexBlock.idx, 0xFD, sizeof(status->currentIndexBlock.idx));
	ims->index[status->currentZoom - MIN_ZOOM_LEVEL].firstBlock = ims->indexHWM;
}

bool ImsAddTile(IMS* ims, NewMapStatus* status, const NewTile* tile, int id)
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

	assert(status->currentZoom >= MIN_ZOOM_LEVEL);
	assert(status->currentZoom <= MAX_ZOOM_LEVEL);
	ui32 i = status->currentZoom - MIN_ZOOM_LEVEL;
	if ((status->tilesAtCurrentZoom == ims->index[i].nx * ims->index[i].ny)
		|| (status->tilesAtCurrentZoom % INDEX_ITEMS_PER_BLOCK == 0))
	{
		if (status->tilesAtCurrentZoom % INDEX_ITEMS_PER_BLOCK)
		{
			assert(ims->indexHWM == ims->index[i].firstBlock + status->tilesAtCurrentZoom / INDEX_ITEMS_PER_BLOCK);
		}
		else
		{
			assert(ims->indexHWM + 1 == ims->index[i].firstBlock + status->tilesAtCurrentZoom / INDEX_ITEMS_PER_BLOCK);
		}
		status->currentIndexBlock.checksum = CalcCRC(status->currentIndexBlock.idx, sizeof(status->currentIndexBlock.idx));
		sdCardWrite(ims->indexHWM, &status->currentIndexBlock, id);
		ims->indexHWM++;
		if (ims->dataHWM <= ims->indexHWM)
			return false;
#ifdef _DEBUG
		memset(status->currentIndexBlock.idx, 0xFD, sizeof(status->currentIndexBlock.idx));
#endif
	}
	return true;
}

void FsCommitIMS(IMS* ims, BlockAddr addr, int id)
{
	ims->status = IMS_READY;
	ims->checksum = 0;
	ims->checksum = CalcCRC(ims, sizeof(*ims));
	assert(sizeof(*ims) <= BLOCK_SIZE);
	ui8 b[BLOCK_SIZE];
	memcpy(b, ims, sizeof(*ims));
	sdCardWrite(addr, b, id);
}

void FsReadTile(BlockAddr addr, ui8* dst, int id)
{
	DecompState s;
	DecompImit(&s, dst);

	ui8 b[BLOCK_SIZE];
	sdCardRead(addr--, b, id);
	ui32 sz = ((NewTile*)b)->size;
	for (ui32 i = sizeof(sz); i < sz; ++i)
	{
		if (0 == (i % BLOCK_SIZE))
			sdCardRead(addr--, b, id);
		DeCompressOne(b[i % BLOCK_SIZE], &s);
	}
}
