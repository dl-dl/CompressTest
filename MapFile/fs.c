#define _CRT_SECURE_NO_WARNINGS
#include "fileio.h"
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

ui32 MapCalcCRC(const void *data, ui32 sz) // TODO: implement proper algorithm
{
 ui32 crc = 0;
 for (ui32 i = 0; i < sz; ++i)
  crc += *((ui8 *)data + i);
 return crc;
}

ui32 MapFindIMS(int x, int y, IMS *dst)
{
 for (ui32 i = 1; i < MAX_NUM_IMS; ++i)
  {
   IMS ims;
   if (!file_read(0, &ims, sizeof(ims)))
    continue;
   if (ims.checksum == MapCalcCRC(&ims, sizeof(ims) - sizeof(ims.checksum)))
    {
     if (PointInRectInt(&ims.coord, x, y))
      {
       *dst = ims;
       return i;
      }
    }
  }
 return 0;
}

TileIndexItem MapFindTile(const IMS *ims, ui8 zoom, ui32 numx, ui32 numy)
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

 TileIndexItem ii;
 ui32 n = dx * ims->index[i].ny + dy;
 if (!file_read(ims->index[i].firstBlock + n * sizeof(ii), &ii, sizeof(ii)))
  return zero;
 return ii;
}

void MapReadTile(BlockAddr addr, ui32 sz, ui8 *dst)
{
 DecompState s;
 DecompImit(&s, dst);

 ui8 b[BLOCK_SIZE];
 for (ui32 i = 0; i < sz; ++i)
  {
   if (0 == (i % BLOCK_SIZE))
    if (file_read(addr, b, BLOCK_SIZE))
     {
      addr += BLOCK_SIZE;
     }
    else
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
