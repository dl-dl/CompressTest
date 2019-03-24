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
       if (cntEqPix >= 0x7F)
        break;
      }
     if (cntEqPix > 2)
      {
       *CompressPtr++ = (cntEqPix << 1) | 0x01;
       *CompressPtr++ = srcCol[y];
       y = y1;
      }
     else if (cntEqPix == 2)
      {
       *CompressPtr++ = srcCol[y] | 0x10;
       y = y1;
      }
     else
      {
		// assert(0 == srcCol[y] & 0x11)
       *CompressPtr++ = srcCol[y];
       y++;
      }
    }
  }
 if (cntEqLine)
  {
   *CompressPtr++ = 0x01;
   *CompressPtr++ = cntEqLine & 0xff;
  }
 return CompressPtr - (ui8 *)dst;
}

void DecompImit(DecompState *s, ui8 *dst)
{
 s->x = s->y = 0;
 s->numEqPixel = 0;
 s->eqString = false;
 s->dst = dst;
}

static inline void PutByte(DecompState *s, ui8 v)
{
 *(s->dst + s->x * TILE_DY / 2 + s->y) = v;
 s->y++;
 if (s->y >= TILE_DY / 2)
  {
   s->y = 0;
   s->x++;
  }
}

static inline void CopyPrevLine(DecompState *s)
{
 ui8 *dstCol = s->dst + s->x * TILE_DY / 2;
 memcpy(dstCol, dstCol - TILE_DY / 2, TILE_DY / 2);
 s->x++;
}

void DeCompressOne(ui8 src, DecompState *s)
{
 if (s->eqString)
  {
   while (src--) // src == repeat count
    CopyPrevLine(s);
   s->eqString = false;
  }
 else if (s->numEqPixel)
  {
   for (ui8 cnt = 0; cnt < s->numEqPixel; cnt++)
    PutByte(s, src);
   s->numEqPixel = 0;
  }
 else
  {
   if (0x01 == src)
    {
     s->eqString = true;
    }
   else if (src & 0x01)
    {
     s->numEqPixel = src >> 1;
    }
   else if (src & 0x10)
    {
     PutByte(s, src & ~0x10);
     PutByte(s, src & ~0x10);
    }
   else
    {
     PutByte(s, src);
    }
  }
}

#if 0
#include <math.h>
//---------------------------------------------------------------------------
// sR, sG and sB (Standard RGB) input range = 0 ? 255
// X, Y and Z output refer to a D65/2° standard illuminant.

void RGB2XYZ(ui8 *bCol, ui8 *gCol, ui8 *rCol)
{
 double var_R = *rCol;
 var_R /= 255;

 double var_G = *gCol;
 var_G /= 255;

 double var_B = *bCol;
 var_B /= 255;

 if (var_R > 0.04045)
  var_R = pow(((var_R + 0.055) / 1.055), 2.4);
 else
  var_R = var_R / 12.92;

 if (var_G > 0.04045)
  var_G = pow(((var_G + 0.055) / 1.055), 2.4);
 else
  var_G = var_G / 12.92;

 if (var_B > 0.04045)
  var_B = pow(((var_B + 0.055) / 1.055), 2.4);
 else
  var_B = var_B / 12.92;

 var_R *= 100;
 var_G *= 100;
 var_B *= 100;

 double Xcolor = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
 double Ycolor = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
 double Zcolor = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;

 int colM = Xcolor / 30;
 Xcolor = colM * 30;

 colM = Ycolor / 30;
 Ycolor = colM * 30;

 colM = Zcolor / 30;
 Zcolor = colM * 30;

 double var_X = Xcolor / 100;
 double var_Y = Ycolor / 100;
 double var_Z = Zcolor / 100;

 var_R = var_X * 3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
 var_G = var_X * -0.9689 + var_Y * 1.8758 + var_Z * 0.0415;
 var_B = var_X * 0.0557 + var_Y * -0.2040 + var_Z * 1.0570;

 if (var_R > 0.0031308)
  var_R = 1.055 * (pow(var_R, (1 / 2.4))) - 0.055;
 else
  var_R = 12.92 * var_R;

 if (var_G > 0.0031308)
  var_G = 1.055 * (pow(var_G, (1 / 2.4))) - 0.055;
 else
  var_G = 12.92 * var_G;

 if (var_B > 0.0031308)
  var_B = 1.055 * (pow(var_B, (1 / 2.4))) - 0.055;
 else
  var_B = 12.92 * var_B;

 *rCol = var_R * 255;
 *gCol = var_G * 255;
 *bCol = var_B * 255;
}
#endif