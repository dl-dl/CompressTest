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

ui32 FsCalcCRC(const void *data, ui32 sz) // TODO: implement proper algorithm
{
 ui32 crc = 0;
 for (ui32 i = 0; i < sz; ++i)
  crc += *((ui8 *)data + i);
 return crc;
}

bool FsFindIMS(int x, int y, IMS *dst)
{
 ui8 b[BLOCK_SIZE];
 for (BlockAddr i = 0; i < NUM_IMS_BLOCKS; ++i)
  {
   if (!SDCardMapRead(i, b, 1))
    return false;
   const IMS *ims = (IMS *)b;
   if (ims->checksum != FsCalcCRC(ims, sizeof(*ims) - sizeof(ims->checksum)))
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
 if (!SDCardMapRead(ims->index[i].firstBlock + offs / INDEX_ITEMS_PER_BLOCK, b, 1))
  return zero;
 const TileIndexBlock *p = (TileIndexBlock *)b;
 if (p->checksum != FsCalcCRC(p->idx, sizeof(p->idx)))
  return zero;
 return p->idx[offs % INDEX_ITEMS_PER_BLOCK];
}

void FsReadTile(BlockAddr addr, ui32 sz, ui8 *dst)
{
 DecompState s;
 DecompImit(&s, dst);

 ui8 b[BLOCK_SIZE];
 for (ui32 i = 0; i < sz; ++i)
  {
   if (0 == (i % BLOCK_SIZE))
    if (!SDCardMapRead(addr--, b, 1))
     {
      assert(0);
      return;
     }
   if (!DeCompressOne(b[i % BLOCK_SIZE], &s))
    {
     assert(0);
     return;
    }
  }
 assert(s.y == 0);
}
