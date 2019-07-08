#include "convert.h"
#include "sizes.h"
#include <string.h>

#define DEV_RED 0x08
#define DEV_GREEN 0x04
#define DEV_BLUE 0x02

void Invert24(const void *src, void *dst)
{
 for (int y = 0; y < TILE_DY; y++)
  {
   const ui8 *srcPtr = (ui8 *)src + y * TILE_DX * 3;
   for (int x = 0; x < TILE_DX; x++)
    {
     ui8 *dstPtr = (ui8 *)dst + (x * TILE_DY + y) * 3;
     dstPtr[0] = *srcPtr++;
     dstPtr[1] = *srcPtr++;
     dstPtr[2] = *srcPtr++;
    }
  }
}

void Invert8(const void *src, void *dst)
{
 for (int y = 0; y < TILE_DY; y++)
  {
   const ui8 *srcRow = (ui8 *)src + y * TILE_DX;
   for (int x = 0; x < TILE_DX; x++)
    {
     *((ui8 *)dst + x * TILE_DY + y) = srcRow[x];
    }
  }
}

void Convert24To8(const void *src, void *dst)
{
 static const ui8 flag[3] = { DEV_RED, DEV_GREEN, DEV_BLUE };
 for (int x = 0; x < TILE_DX; x++)
  {
   ui8 *dstCol = (ui8 *)dst + x * TILE_DY;
   const ui8 *srcPtr = (ui8 *)src + (x * TILE_DY) * 3;
   for (int y = 0; y < TILE_DY; y++)
    {
     ui8 b = 0;
     for (int i = 0; i < 3; ++i)
      if (*srcPtr++)
       b |= flag[i];
     dstCol[y] = b;
    }
  }
}

void Convert8To24(const void *src, void *dst)
{
 static const ui8 flag[3] = { DEV_RED, DEV_GREEN, DEV_BLUE };
 for (int x = 0; x < TILE_DX; x++)
  {
   const ui8 *srcCol = (ui8 *)src + x * TILE_DY;
   for (int y = 0; y < TILE_DY; y++)
    {
     ui8 *dstPtr = (ui8 *)dst + (x * TILE_DY + y) * 3;
     for (int i = 0; i < 3; ++i)
      dstPtr[i] = (srcCol[y] & flag[i]) ? 0xFF : 0;
    }
  }
}

void Convert8To4(const void *src, void *dst)
{
 for (int x = 0; x < TILE_DX; x++)
  {
   const ui8 *srcCol = (ui8 *)src + x * TILE_DY;
   ui8 *dstCol = (ui8 *)dst + x * TILE_DY / 2;
   for (int y = 0; y < TILE_DY / 2; y++)
    dstCol[y] = (srcCol[y * 2 + 1] << 4) | srcCol[y * 2];
  }
}

void Convert4To24(const void *src, void *dst)
{
 static const ui8 flag[6] = { DEV_RED << 4, DEV_GREEN << 4, DEV_RED << 4, DEV_RED, DEV_GREEN, DEV_BLUE };
 for (int x = 0; x < TILE_DX; x++)
  {
   const ui8 *srcCol = (ui8 *)src + x * TILE_DY / 2;
   ui8 *dstCol = (ui8 *)dst + x * TILE_DY * 3;
   for (int y = 0; y < TILE_DY / 2; y++)
    {
     for (int i = 0; i < 6; ++i)
      *dstCol++ = (srcCol[y] & flag[i]) ? 0xFF : 0;
    }
  }
}

//---------------------------------------------------------------------------
unsigned int Compress4BitBuffer(const void *src, void *dst)
{
 ui8 *CompressPtr = (ui8 *)dst;
 ui8 cntEqLine = 0;
 for (int x = 0; x < TILE_DX; x++)
  {
   const ui8 *srcCol = (ui8 *)src + x * TILE_DY / 2;
   if (x > 0 && memcmp(srcCol, srcCol - TILE_DY / 2, TILE_DY / 2) == 0)
    {
     cntEqLine++;
     if (cntEqLine < 0xFF)
      continue;
     else
      x++;
    }

   if (cntEqLine)
    {
     *CompressPtr++ = 0x01;
     *CompressPtr++ = cntEqLine;
     x--;
     cntEqLine = 0;
     continue;
    }

   for (int y = 0; y < TILE_DY / 2;)
    {
     ui8 cntEqPix = 1;
     int y1 = y + 1;
     while (y1 < TILE_DY / 2)
      {
       if (srcCol[y1] != srcCol[y])
        break;
       cntEqPix++;
       y1++;
       if (cntEqPix > 0x7F)
        break;
      }
     const ui8 c = srcCol[y] & ~0x11; // & ~0x11: in case of bogus source
     if (cntEqPix > 2)
      {
       *CompressPtr++ = ((cntEqPix - 2) << 1) | 0x01;
       *CompressPtr++ = c;
       y = y1;
      }
     else if (cntEqPix == 2)
      {
       *CompressPtr++ = c | 0x10;
       y = y1;
      }
     else
      {
       *CompressPtr++ = c;
       y++;
      }
    }
  }
 if (cntEqLine)
  {
   *CompressPtr++ = 0x01;
   *CompressPtr++ = cntEqLine;
  }
 return CompressPtr - (ui8 *)dst;
}
