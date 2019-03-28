#include "types.h"
#include "graph.h"
#include "sizes.h"
#include "screen.h"
#include "devfont.h"
#include <string.h>

static const ui8 colorHi[8] = { 0, 0x20, 0x40, 0x80, 0x60, 0xC0, 0xA0, 0xE0 };
static const ui8 colorLo[8] = { 0, 0x02, 0x04, 0x08, 0x06, 0x0C, 0x0A, 0x0E };
static const ui8 colorHL[8] = { 0, 0x22, 0x44, 0x88, 0x66, 0xCC, 0xAA, 0xEE };

void DisplayClear(ui8 color)
{
 memset(&Screen, color | (color << 4), sizeof(Screen));
}

void DisplayPixel(int x, int y, ui8 c)
{
 if ((unsigned)x >= SCREEN_DX)
  return;
 if ((unsigned)y >= SCREEN_DY)
  return;
 c &= 0x07;
 ui8 b = Screen[x].pix[y / 2];
 Screen[x].pix[y / 2] = (y % 2) ? ((b & 0xF0) | colorLo[c]) : ((b & 0x0F) | colorHi[c]);
}

static void VLine(int x, int y, int height, ui8 c)
{
 ui8 *ptr = &Screen[x].pix[y / 2];

 if (y % 2)
  {
   *ptr = (*ptr & 0xF0) | colorLo[c];
   ptr++;
   height--;
  }

 for (ui16 cnt = height / 2; cnt--;)
  *ptr++ = colorHL[c];

 if (height % 2)
  *ptr = ((*ptr & 0x0F) | colorHi[c]);
}

void DisplayFillRect(int left, int top, int width, int height, ui8 color)
{
 if (height <= 0)
  return;
 if (width <= 0)
  return;
 if (left + width > SCREEN_DX)
  return;
 if (top + height > SCREEN_DY)
  return;

 while (width--)
  VLine(left + width, top, height, color);
}

static inline int intAbs(int i)
{
 return i >= 0 ? i : -i;
}

void DisplayLine(int x0, int y0, int x1, int y1, ui8 color)
{
 const int dx = intAbs(x1 - x0), sx = x0 < x1 ? 1 : -1;
 const int dy = -intAbs(y1 - y0), sy = y0 < y1 ? 1 : -1;
 int err = dx + dy, e2;
 for (;;)
  {
   DisplayPixel(x0, y0, color);
   if (x0 == x1 && y0 == y1)
    break;
   e2 = 2 * err;
   if (e2 >= dy)
    {
     err += dy;
     x0 += sx;
    }
   if (e2 <= dx)
    {
     err += dx;
     y0 += sy;
    }
  }
}

void DisplayCircle(int xm, int ym, int r, ui8 color)
{
 if (r <= 0)
  return;
 int x = -r, y = 0, err = 2 - 2 * r;
 do
  {
   DisplayPixel(xm - x, ym + y, color);
   DisplayPixel(xm - y, ym - x, color);
   DisplayPixel(xm + x, ym - y, color);
   DisplayPixel(xm + y, ym + x, color);
   r = err;
   if (r > x)
    err += ++x * 2 + 1;
   if (r <= y)
    err += ++y * 2 + 1;
  }
 while (x < 0);
}

void CopyTileToScreen(const void *tile, int x, int y)
{
 for (int i = 0; i < TILE_DX; ++i)
  {
   int ii = i + x;
   if ((ii >= 0) && (ii < SCREEN_DX))
    for (int j = 0; j < TILE_DY / 2; ++j)
     {
      int jj = j + y;
      if ((jj >= 0) && (jj < SCREEN_DY / 2))
       Screen[ii].pix[SCREEN_DY / 2 - 1 - jj] = *((ui8 *)tile + i * TILE_DY / 2 + j);
     }
  }
}

void DisplayRainbow()
{
 for (ui16 x = 0; x < SCREEN_DX; x++)
  for (ui16 y = 0; y < SCREEN_DY / 2; y++)
   {
    Screen[x].pix[y] = colorHL[y / (SCREEN_DY / 16)];
   }
}

//===========================================

static const DevFont *const Fonts[] = {
 &font32x25,
 &font24x19,
 0
};

static const unsigned char *FindChr(const DevFont *f, ui32 c)
{
 const ui32 numBlocks = sizeof(f->block) / sizeof(*(f->block));
 for (ui32 i = 0; i < numBlocks; ++i)
  {
   if ((c >= f->block[i].base) && (c <= f->block[i].base + f->block[i].sz))
    return &f->block[i].sym[(c - f->block[i].base) * (f->maxW * f->bytesH + 1)];
  }
 return f->block[0].sym;
}

static unsigned int DisplayChr(ui32 chr, ui32 x, ui32 y, const DevFont *f, ui8 color)
{
 const unsigned char *c = FindChr(f, chr);
 y += f->h;
 for (ui32 i = 0; i < c[0]; ++i)
  {
   for (ui32 j = 0; j < f->h; ++j)
    {
     if (c[i * f->bytesH + 1 + j / 8] & (1 << (j % 8)))
      DisplayPixel(x + i, y - j, color);
    }
  }
 return c[0];
}

void DisplayText(const char *s, ui32 x, ui32 y, ui8 fontType, ui8 color)
{
 const DevFont *f = Fonts[fontType];
 if (f == 0)
  return;

 for (ui32 i = 0; s[i]; ++i)
  x += DisplayChr(s[i], x, y, f, color);
}

void DisplayTextW(const ui16 *s, ui32 x, ui32 y, ui8 fontType, ui8 color)
{
 const DevFont *f = Fonts[fontType];
 if (f == 0)
  return;

 for (ui32 i = 0; s[i]; ++i)
  x += DisplayChr(s[i], x, y, f, color);
}
