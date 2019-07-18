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

void MapFindIMS(int x, int y, ExtIMS *dst)
{
 dst->fname[0] = 0;
 if (!file_open_dir())
  {
   assert(0);
   return;
  }
 const char *fname;
 while ((fname = file_read_dir()))
  {
   if (file_open(fname, false))
    {
     IMS ims;
     bool res = file_read(0, &ims, sizeof(ims));
     file_close();

     if (res)
      if ((ims.version == 2) && (ims.checksum == MapCalcCRC(&ims, sizeof(ims) - sizeof(ims.checksum))))
       if (PointInRectInt(&ims.coord, x, y))
        {
         if (strlen(fname) < sizeof(dst->fname))
          {
           dst->ims = ims;
           strcpy(dst->fname, fname);
           break;
          }
        }
    }
  }
 file_close_dir();
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
 if (!file_read(ims->index[i].firstBlock + n * sizeof(ii.addr), &ii, sizeof(ii)))
  return zero;
 return ii;
}

void MapReadTile(FileAddr addr, ui32 sz, ui8 *dst)
{
 DecompState s;
 DecompImit(&s, dst);

 ui8 b[512];
 for (ui32 i = 0; i < sz; ++i)
  {
   if (0 == (i % sizeof(b)))
    {
     if (!file_read(addr + i, b, (sz - i > sizeof(b)) ? sizeof(b) : (sz - i)))
      {
       assert(0);
       return;
      }
    }
   if (!DeCompressOne(b[i % sizeof(b)], &s))
    {
     assert(0);
     return;
    }
  }
 assert(s.y == 0);
}
