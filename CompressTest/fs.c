#include "sd.h"
#include "fs.h"
#include "coord.h"
#include "convert.h"
#include <string.h>
#ifdef _MSC_VER
#include <assert.h>
#else
#include "sound.h"
#define assert(expression) (void)((!!(expression)) || (Sound(10, 100), 0))
//#define assert(expression) ((void)0)
#endif

static ui32 CalcCRC(const void *data, ui32 sz) // TODO: implement proper algorithm
{
 ui32 crc = 0;
 for (ui32 i = 0; i < sz; ++i)
  crc += *((ui8 *)data + i);
 return crc;
}

void FsFormat()
{
 ui8 b[BLOCK_SIZE];
 memset(b, 0, sizeof(b));
 for (BlockAddr i = 0; i < NUM_IMS_BLOCKS; ++i)
  SDCardWrite(i, b, 1);
}

static bool FindFirstEmptyIMS(BlockAddr *dataHWM, BlockAddr *indexHWM, BlockAddr *addr)
{
 ui8 b[BLOCK_SIZE];
 *indexHWM = NUM_IMS_BLOCKS; // init value for empty filel system
 *dataHWM = SDCardSize() - 1;
 for (BlockAddr i = 0; i < NUM_IMS_BLOCKS; ++i)
  {
   if (!SDCardRead(i, b, 1))
    return false;
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

BlockAddr FsFreeSpace()
{
 BlockAddr dataHWM, indexHWM, addr;
 if (!FindFirstEmptyIMS(&dataHWM, &indexHWM, &addr))
  return SDCardSize() - 2;
 return dataHWM - indexHWM;
}

bool FsNewIMS(IMS *ims, BlockAddr *addr, const RectInt *coord)
{
 BlockAddr dataHWM, indexHWM;
 if (!FindFirstEmptyIMS(&dataHWM, &indexHWM, addr))
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
   ims->index[i].left = ScaleDownCoord(ims->coord.left, z);
   ims->index[i].top = ScaleDownCoord(ims->coord.top, z);
   ui32 dx = ScaleDownCoord(ims->coord.right, z) - ims->index[i].left + 1;
   ui32 dy = ScaleDownCoord(ims->coord.bottom, z) - ims->index[i].top + 1;
   ims->index[i].nx = dx;
   ims->index[i].ny = dy;
   ui32 numBlocks = (dx * dy) / INDEX_ITEMS_PER_BLOCK;
   if ((dx * dy) % INDEX_ITEMS_PER_BLOCK)
    numBlocks++;
   ims->indexHWM += numBlocks;
  }

 return true;
}

bool FsFindIMS(int x, int y, IMS *dst)
{
 ui8 b[BLOCK_SIZE];
 for (BlockAddr i = 0; i < NUM_IMS_BLOCKS; ++i)
  {
   if (!SDCardRead(i, b, 1))
    return false;
   const IMS *ims = (IMS *)b;
   if (ims->checksum != CalcCRC(ims, sizeof(*ims) - sizeof(ims->checksum)))
    return false;
   if (IMS_EMPTY == ims->status)
    return false;
   if (IMS_READY == ims->status)
    {
     if (PointInRectInt(&ims->coord, x, y))
      {
       *dst = *ims;
       return true;
      }
    }
  }
 return false;
}

TileIndexItem FsFindTile(const IMS *ims, ui8 zoom, ui32 numx, ui32 numy)
{
 assert(zoom >= MIN_ZOOM_LEVEL);
 assert(zoom <= MAX_ZOOM_LEVEL);
 const TileIndexItem zero = { 0, 0 };
 ui32 i = zoom - MIN_ZOOM_LEVEL;
 if (0 == ims->index[i].firstBlock)
  return zero;

 ui32 dx = numx - ims->index[i].left;
 ui32 dy = numy - ims->index[i].top;
 if (dx >= ims->index[i].nx || dy >= ims->index[i].ny)
  return zero;

 ui32 offs = dx * ims->index[i].ny + dy;
 ui8 b[BLOCK_SIZE];
 if (!SDCardRead(ims->index[i].firstBlock + offs / INDEX_ITEMS_PER_BLOCK, b, 1))
  return zero;
 const TileIndexBlock *p = (TileIndexBlock *)b;
 if (p->checksum != CalcCRC(p->idx, sizeof(p->idx)))
  return zero;
 return p->idx[offs % INDEX_ITEMS_PER_BLOCK];
}

void ImsNextZoom(IMS *ims, NewMapStatus *status, ui8 zoom)
{
 assert(zoom >= MIN_ZOOM_LEVEL);
 assert(zoom <= MAX_ZOOM_LEVEL);
 status->currentZoom = zoom;
 status->tilesAtCurrentZoom = 0;
 memset(status->currentIndexBlock.idx, 0xFD, sizeof(status->currentIndexBlock.idx));
 ims->index[status->currentZoom - MIN_ZOOM_LEVEL].firstBlock = ims->indexHWM;
}

bool ImsAddTile(IMS *ims, NewMapStatus *status, const ui8 *tile, ui32 sz)
{
 assert(ims->dataHWM >= ims->indexHWM);
 TileIndexItem *ii = &status->currentIndexBlock.idx[status->tilesAtCurrentZoom % INDEX_ITEMS_PER_BLOCK];
 ii->addr = ims->dataHWM;
 ii->sz = sz;
 for (ui32 written = 0; written < sz; written += BLOCK_SIZE) // write data
  {
   SDCardWrite(ims->dataHWM, tile + written, 1);
   ims->dataHWM--;
   if (ims->dataHWM <= ims->indexHWM)
    return false;
  }
 status->tilesAtCurrentZoom++;

 assert(status->currentZoom >= MIN_ZOOM_LEVEL);
 assert(status->currentZoom <= MAX_ZOOM_LEVEL);
 ui32 i = status->currentZoom - MIN_ZOOM_LEVEL;
 if ((status->tilesAtCurrentZoom == ims->index[i].nx * ims->index[i].ny) || (status->tilesAtCurrentZoom % INDEX_ITEMS_PER_BLOCK == 0))
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
   SDCardWrite(ims->indexHWM, &status->currentIndexBlock, 1);
   ims->indexHWM++;
   if (ims->dataHWM <= ims->indexHWM)
    return false;
#ifdef _DEBUG
   memset(status->currentIndexBlock.idx, 0xFD, sizeof(status->currentIndexBlock.idx));
#endif
  }
 return true;
}

bool FsCommitIMS(IMS *ims, BlockAddr addr)
{
 ims->status = IMS_READY;
 ims->checksum = CalcCRC(ims, sizeof(*ims) - sizeof(ims->checksum));
 assert(sizeof(*ims) <= BLOCK_SIZE);
 ui8 b[BLOCK_SIZE];
 *(IMS *)b = *ims;
 return SDCardWrite(addr, b, 1);
}

void FsReadTile(BlockAddr addr, ui32 sz, ui8 *dst)
{
 DecompState s;
 DecompImit(&s, dst);

 ui8 b[BLOCK_SIZE];
 for (ui32 i = 0; i < sz; ++i)
  {
   if (0 == (i % BLOCK_SIZE))
    if (!SDCardRead(addr--, b, 1))
     {
      assert(0);
      return;
     }
   DeCompressOne(b[i % BLOCK_SIZE], &s);
  }
 assert(s.y == 0);
}
