#define _CRT_SECURE_NO_WARNINGS
#include "types.h"
#include "fsinit.h"
#include "fs.h"
#include "sizes.h"
#include "sd.h"
#include "convert2.h"
#ifdef _DEBUG
#include "convert.h"
#endif

#include "lodepng.h"

#include <malloc.h>
#include <assert.h>
#include <stdio.h>

struct NewTile
{
 ui32 size;
 ui8 *data;
};

void FsFormat()
{
 ui8 b[BLOCK_SIZE];
 memset(b, 0, sizeof(b));
 for (BlockAddr i = 0; i < NUM_IMS_BLOCKS; ++i)
  SDCardMapWrite(i, b, 1);
}

static bool FindFirstEmptyIMS(BlockAddr *dataHWM, BlockAddr *indexHWM, BlockAddr *addr)
{
 ui8 b[BLOCK_SIZE];
 *indexHWM = NUM_IMS_BLOCKS; // init value for empty file system
 *dataHWM = MAP_SIZE - 1;
 for (BlockAddr i = 0; i < NUM_IMS_BLOCKS; ++i)
  {
   if (!SDCardMapRead(i, b, 1))
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

static ui32 FsFreeSpace()
{
 BlockAddr dataHWM, indexHWM, addr;
 if (!FindFirstEmptyIMS(&dataHWM, &indexHWM, &addr))
  return MAP_SIZE - 2;
 return dataHWM - indexHWM;
}

static bool ImsAddTile(IMS *ims, NewMapStatus *status, const ui8 *tile, ui32 sz)
{
 assert(ims->dataHWM >= ims->indexHWM);
 TileIndexItem *ii = &status->currentIndexBlock.idx[status->tilesAtCurrentZoom % INDEX_ITEMS_PER_BLOCK];
 ii->addr = ims->dataHWM;
 ii->sz = sz;
 for (ui32 written = 0; written < sz; written += BLOCK_SIZE) // write data
  {
   SDCardMapWrite(ims->dataHWM, tile + written, 1);
   ims->dataHWM--;
   if (ims->dataHWM <= ims->indexHWM)
    {
     assert(0);
     return false;
    }
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
   status->currentIndexBlock.checksum = FsCalcCRC(status->currentIndexBlock.idx, sizeof(status->currentIndexBlock.idx));
   SDCardMapWrite(ims->indexHWM, &status->currentIndexBlock, 1);
   ims->indexHWM++;
   if (ims->dataHWM <= ims->indexHWM)
    {
     assert(0);
     return false;
    }
#ifdef _DEBUG
   memset(status->currentIndexBlock.idx, 0xFD, sizeof(status->currentIndexBlock.idx));
#endif
  }
 return true;
}

static bool FsCommitIMS(IMS *ims, BlockAddr addr)
{
 ims->status = IMS_READY;
 ims->checksum = FsCalcCRC(ims, sizeof(*ims) - sizeof(ims->checksum));
 assert(sizeof(*ims) <= BLOCK_SIZE);
 ui8 b[BLOCK_SIZE];
 *(IMS *)b = *ims;
 return SDCardMapWrite(addr, b, 1);
}

static bool FsNewIMS(IMS *ims, BlockAddr *addr, const RectInt *coord)
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
   ui32 numBlocks = (dx * dy - 1) / INDEX_ITEMS_PER_BLOCK + 1;
   ims->indexHWM += numBlocks;
  }

 return true;
}

static NewTile getTile(int x, int y, ui8 z, const char *region)
{
 char path[1024];
 sprintf(path, "C:\\tmp\\%s\\%u\\%u\\%u.png", region, z, x, y);
 NewTile t = { 0, 0 };

 FILE *f = fopen(path, "rb");
 if (!f)
  {
   assert(0);
   return t;
  }
 fseek(f, 0, SEEK_END);
 size_t pngsize = ftell(f);
 fseek(f, 0, SEEK_SET);
 ui8 *png = (unsigned char *)malloc(pngsize);
 if (NULL == png)
  return t;
 fread(png, 1, pngsize, f);
 fclose(f);

 unsigned width, height;
 ui8 *b;
 unsigned error = lodepng_decode_memory(&b, &width, &height, png, pngsize, LCT_RGB, 8);
 if (error)
  return t;
 free(png);

 ui8 *p24 = (ui8 *)malloc(TILE_DX * TILE_DY * 3);
 Invert24(b, p24);
 free(b);

 ui8 *p8 = (ui8 *)malloc(TILE_DX * TILE_DY);
 Convert24To8(p24, p8);
 free(p24);
 ui8 *p4 = (ui8 *)malloc(TILE_DX * TILE_DY / 2);
 Convert8To4(p8, p4);
 free(p8);
 t.data = (ui8 *)malloc(TILE_DX * TILE_DY / 2 + BLOCK_SIZE); // round up to BLOCK_SIZE
 t.size = Compress4BitBuffer(p4, t.data);
#ifdef CHECK_DECOMPRESS
 {
  ui8 *tile = (ui8 *)malloc(TILE_DX * TILE_DY / 2);
  DecompState s;
  DecompImit(&s, tile);
  for (ui32 i = 0; i < t.size; ++i)
   DeCompressOne(t.data[i], &s);
  assert(0 == memcmp(p4, tile, TILE_DX * TILE_DY / 2));
  free(tile);
 }
#endif
 free(p4);
 return t;
}

static void forgetTile(NewTile t)
{
 free(t.data);
}

static void ImsNextZoom(IMS *ims, NewMapStatus *status, ui8 zoom)
{
 assert(zoom >= MIN_ZOOM_LEVEL);
 assert(zoom <= MAX_ZOOM_LEVEL);
 status->currentZoom = zoom;
 status->tilesAtCurrentZoom = 0;
 memset(status->currentIndexBlock.idx, 0xFD, sizeof(status->currentIndexBlock.idx));
 ims->index[status->currentZoom - MIN_ZOOM_LEVEL].firstBlock = ims->indexHWM;
}

void FsInit()
{
 IMS ims;
 const int NUM_REG = 2;
 RectInt r[NUM_REG];
 const char *region[NUM_REG] = {
  "cher",
  "aur"
 };
 /*
 r[0].left = lon2tilex(38.1f, MAX_ZOOM_LEVEL) / TILE_DX;
 r[0].top = lat2tiley(56.1f, MAX_ZOOM_LEVEL) / TILE_DY;
 r[0].right = lon2tilex(38.6f, MAX_ZOOM_LEVEL) / TILE_DX;
 r[0].bottom = lat2tiley(55.95f, MAX_ZOOM_LEVEL) / TILE_DY;
*/
 r[0].left = lon2tilex(37.1f, MAX_ZOOM_LEVEL) / TILE_DX;
 r[0].top = lat2tiley(56.1f, MAX_ZOOM_LEVEL) / TILE_DY;
 r[0].right = lon2tilex(38.9f, MAX_ZOOM_LEVEL) / TILE_DX;
 r[0].bottom = lat2tiley(55.4f, MAX_ZOOM_LEVEL) / TILE_DY;

 r[1].left = lon2tilex(-79.50f, MAX_ZOOM_LEVEL) / TILE_DX;
 r[1].top = lat2tiley(44.01f, MAX_ZOOM_LEVEL) / TILE_DY;
 r[1].right = lon2tilex(-79.46f, MAX_ZOOM_LEVEL) / TILE_DX;
 r[1].bottom = lat2tiley(43.99f, MAX_ZOOM_LEVEL) / TILE_DY;

 for (int i = 0; i < NUM_REG - 1; ++i)
  {
   BlockAddr addr;
   FsNewIMS(&ims, &addr, r + i);
   for (ui8 z = CURRENT_MAP_MIN_ZOOM; z <= CURRENT_MAP_MAX_ZOOM; ++z)
    {
     NewMapStatus status;
     ImsNextZoom(&ims, &status, z);
     const ImsIndexDescr *idx = &ims.index[z - MIN_ZOOM_LEVEL];
     for (ui32 x = 0; x < idx->nx; ++x)
      for (ui32 y = 0; y < idx->ny; ++y)
       {
        NewTile tile = getTile(idx->left + x, idx->top + y, z, region[i]);
        assert(tile.data);
        ImsAddTile(&ims, &status, tile.data, tile.size);
        forgetTile(tile);
       }
    }
   FsCommitIMS(&ims, addr);
  }
 ui32 sz = FsFreeSpace();
}
