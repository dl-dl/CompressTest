#define _CRT_SECURE_NO_WARNINGS
#include "types.h"
#include "fsinit.h"
#include "fs.h"
#include "sizes.h"
#include "fileiostd.h"
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

static bool ImsAddTile(IMS *ims, NewMapStatus *status, const ui8 *tile, ui32 sz)
{
 assert(status->currentZoom >= MIN_ZOOM_LEVEL);
 assert(status->currentZoom <= MAX_ZOOM_LEVEL);
 TileIndexItem ii;
 ii.addr = ims->dataHWM;
 ii.sz = sz;
 if (!file_write(ims->dataHWM, tile, sz))
  return false;

 ui32 i = status->currentZoom - MIN_ZOOM_LEVEL;
 if (!file_write(ims->index[i].firstBlock + status->tilesAtCurrentZoom * sizeof(ii), &ii, sizeof(ii)))
  return false;

 ims->dataHWM += sz;
 status->tilesAtCurrentZoom++;
 return true;
}

static bool MapCommitIMS(IMS *ims)
{
 ims->checksum = MapCalcCRC(ims, sizeof(*ims) - sizeof(ims->checksum));
 return file_write(0, ims, sizeof(*ims));
}

static void MapNewIMS(IMS *ims, const RectInt *coord)
{
 memset(ims, 0, sizeof(*ims));
 ims->version = 1;
 ims->coord = *coord;
 FileAddr hwm = sizeof(IMS);

 for (ui8 z = MIN_ZOOM_LEVEL; z <= MAX_ZOOM_LEVEL; ++z)
  {
   int i = z - MIN_ZOOM_LEVEL;
   ims->index[i].firstBlock = hwm;
   ims->index[i].left = ScaleDownCoord(ims->coord.left, z);
   ims->index[i].top = ScaleDownCoord(ims->coord.top, z);
   ui32 dx = ScaleDownCoord(ims->coord.right, z) - ims->index[i].left + 1;
   ui32 dy = ScaleDownCoord(ims->coord.bottom, z) - ims->index[i].top + 1;
   ims->index[i].nx = dx;
   ims->index[i].ny = dy;
   hwm += dx * dy * sizeof(TileIndexItem);
  }
 ims->dataHWM = hwm;
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
//#define CHECK_DECOMPRESS
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
}

void MapFileInit()
{
 IMS ims;
 RectInt r;

 const char region[] = "cher";
 r.left = lon2tilex(38.1f, MAX_ZOOM_LEVEL) / TILE_DX;
 r.top = lat2tiley(56.1f, MAX_ZOOM_LEVEL) / TILE_DY;
 r.right = lon2tilex(38.6f, MAX_ZOOM_LEVEL) / TILE_DX;
 r.bottom = lat2tiley(55.95f, MAX_ZOOM_LEVEL) / TILE_DY;
 /*
 const char region[] = "cher";
 r.left = lon2tilex(37.1f, MAX_ZOOM_LEVEL) / TILE_DX;
 r.top = lat2tiley(56.1f, MAX_ZOOM_LEVEL) / TILE_DY;
 r.right = lon2tilex(38.9f, MAX_ZOOM_LEVEL) / TILE_DX;
 r.bottom = lat2tiley(55.4f, MAX_ZOOM_LEVEL) / TILE_DY;

 const char region[] = "aur";
 r.left = lon2tilex(-79.50f, MAX_ZOOM_LEVEL) / TILE_DX;
 r.top = lat2tiley(44.01f, MAX_ZOOM_LEVEL) / TILE_DY;
 r.right = lon2tilex(-79.46f, MAX_ZOOM_LEVEL) / TILE_DX;
 r.bottom = lat2tiley(43.99f, MAX_ZOOM_LEVEL) / TILE_DY;
*/

 file_open("f0.bin", true);

 MapNewIMS(&ims, &r);
 for (ui8 z = CURRENT_MAP_MIN_ZOOM; z <= CURRENT_MAP_MAX_ZOOM; ++z)
  {
   NewMapStatus status;
   ImsNextZoom(&ims, &status, z);
   printf("ZOOM %u\n", z);
   const ImsIndexDescr *idx = &ims.index[z - MIN_ZOOM_LEVEL];
   for (ui32 x = 0; x < idx->nx; ++x)
    for (ui32 y = 0; y < idx->ny; ++y)
     {
      NewTile tile = getTile(idx->left + x, idx->top + y, z, region);
      assert(tile.data);
      ImsAddTile(&ims, &status, tile.data, tile.size);
      forgetTile(tile);
     }
  }
 MapCommitIMS(&ims);

 file_close();
}
