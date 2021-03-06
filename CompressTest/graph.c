#include "graph.h"
#include "types.h"
#include "sizes.h"
#include "devfont.h"

extern ui8 Screen[SCREEN_DX][SCREEN_DY / 2];
extern ui8 ScreenChange[SCREEN_DX];

static const ui8 colorLo[8] = { 0x00, 0x20, 0x40, 0x80, 0x60, 0xC0, 0xA0, 0xE0 };
static const ui8 colorHi[8] = { 0x00, 0x02, 0x04, 0x08, 0x06, 0x0C, 0x0A, 0x0E };
static const ui8 colorHL[8] = { 0x00, 0x22, 0x44, 0x88, 0x66, 0xCC, 0xAA, 0xEE };

//---------------------------------------------------------------------------------------------------
void DisplayPixel(ui16 x, ui16 y, ui8 color)
{
 if (x >= SCREEN_DX || y >= SCREEN_DY)
  return;
 color &= 0x07;

 ui8 *ptr = &Screen[x][y / 2];

 if (y % 2)
  *ptr = (*ptr & 0xF0) | colorHi[color];
 else
  *ptr = (*ptr & 0x0F) | colorLo[color];

 ScreenChange[x] = 1;
}
//---------------------------------------------------------------------------------------------------
static void DisplayVLine(ui16 x, ui16 y, ui16 height, ui8 color)
{
 if (height == 0)
  return;
 if (x >= SCREEN_DX || y >= SCREEN_DY)
  return;

 if (y + height >= SCREEN_DY)
  height = SCREEN_DY - y - 1;

 color &= 0x07;

 ui8 *ptr = &(Screen[x][y / 2]);

 if (y % 2)
  {
   *ptr = ((*ptr & 0xF0) | colorHi[color]);
   ptr++;
   height--;
  }

 for (ui16 cnt = height / 2; cnt--;)
  *ptr++ = colorHL[color];

 if (height % 2)
  *ptr = ((*ptr & 0x0F) | colorLo[color]);

 ScreenChange[x] = 1;
}
//---------------------------------------------------------------------------------------------------
static inline int intAbs(int i)
{
 return i >= 0 ? i : -i;
}
//---------------------------------------------------------------------------------------------------
void DisplayLine(int x0, int y0, int x1, int y1, ui8 color)
{
 const int dx = intAbs(x1 - x0), sx = x0 < x1 ? 1 : -1;
 const int dy = -intAbs(y1 - y0), sy = y0 < y1 ? 1 : -1;
 int err = dx + dy;
 for (;;)
  {
   DisplayPixel(x0, y0, color);
   if (x0 == x1 && y0 == y1)
    break;
   int e2 = 2 * err;
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
//---------------------------------------------------------------------------------------------------
void DisplayRect(int x0, int y0, int width, int height, ui8 color)
{
 DisplayLine(x0, y0, x0, y0 + height, color);
 DisplayLine(x0, y0 + height, x0 + width, y0 + height, color);
 DisplayLine(x0 + width, y0, x0 + width, y0 + height, color);
 DisplayLine(x0, y0, x0 + width, y0, color);
}
//---------------------------------------------------------------------------------------------------
void DisplayFillRect(ui16 left, ui16 top, ui16 width, ui16 height, ui8 color)
{
 if (left + width >= SCREEN_DX || top + height >= SCREEN_DY)
  return;
 color &= 0x07;

 while (width--)
  DisplayVLine(left + width, top, height, color);
}
//---------------------------------------------------------------------------------------------------
void DisplayRomb(ui16 x0, ui16 y0, ui16 widthHeight, ui8 color)
{
 ui16 len = widthHeight / 2;
 DisplayLine(x0 - len, y0, x0, y0 + len, color);
 DisplayLine(x0, y0 + len, x0 + len, y0, color);
 DisplayLine(x0, y0 - len, x0 + len, y0, color);
 DisplayLine(x0 - len, y0, x0, y0 - len, color);
}
//---------------------------------------------------------------------------------------------------
void DisplayFillRomb(ui16 x0, ui16 y0, ui16 widthHeight, ui8 color)
{
 color &= 0x07;

 ui16 len = widthHeight / 2;
 ui16 cnt = 0;
 while (len > 0)
  {
   DisplayVLine(x0 + cnt, y0 - len, len * 2, color);
   DisplayVLine(x0 - cnt, y0 - len, len * 2, color);
   cnt++;
   len--;
  }
}
//---------------------------------------------------------------------------------------------------
void DisplayCircle(int x0, int y0, int r, ui8 color)
{
 int x = -r, y = 0, err = 2 - 2 * r;
 do
  {
   DisplayPixel(x0 - x, y0 + y, color);
   DisplayPixel(x0 - y, y0 - x, color);
   DisplayPixel(x0 + x, y0 - y, color);
   DisplayPixel(x0 + y, y0 + x, color);
   int e2 = err;
   if (e2 <= y)
    err += ++y * 2 + 1;
   if (e2 > x)
    err += ++x * 2 + 1;
  }
 while (x < 0);
}
//---------------------------------------------------------------------------------------------------
void DisplayFillCircle(int x0, int y0, int r, ui8 color)
{
 int x = -r, y = 0, err = 2 - 2 * r;
 do
  {
   int e2 = err;
   if (e2 <= y)
    err += ++y * 2 + 1;
   if (e2 > x)
    {
     DisplayVLine(x0 - x, y0 - y, 2 * y + 1, color);
     DisplayVLine(x0 + x, y0 - y, 2 * y + 1, color);
     err += ++x * 2 + 1;
    }
  }
 while (x <= 0);
}
//---------------------------------------------------------------------------------------------------
static void swap(int *x1, int *x2, int *y1, int *y2)
{
 int tmp = *x2;
 *x2 = *x1;
 *x1 = tmp;
 tmp = *y2;
 *y2 = *y1;
 *y1 = tmp;
}
//---------------------------------------------------------------------------------------------------
void DisplayTrian(int x0, int y0, int x1, int y1, int x2, int y2, ui8 color)
{
 float d0, d1;
 float yL0, yL1;

 if (x0 > x1)
  swap(&x0, &x1, &y0, &y1);
 if (x1 > x2)
  {
   swap(&x1, &x2, &y1, &y2);
   if (x0 > x1)
    swap(&x0, &x1, &y0, &y1);
  }
 if (x2 == x0)
  return;
 d0 = y2 - y0;
 d0 /= x2 - x0;
 yL0 = y0;
 if (x1 != x0)
  {
   d1 = y1 - y0;
   d1 /= x1 - x0;
   yL1 = y0;
  }
 DisplayPixel(x0, y0, color);
 for (; x0 < x2; x0++)
  {
   yL0 += d0;
   if (x0 == x1)
    {
     yL1 = y1;
     d1 = y2 - y1;
     d1 /= x2 - x1;
    }
   else
    {
     yL1 += d1;
    }
   if (yL0 > yL1)
    DisplayVLine(x0, yL1, yL0 - yL1, color);
   else
    DisplayVLine(x0, yL0, yL1 - yL0, color);
  }
}
//---------------------------------------------------------------------------------------------------
void DisplayRainbow()
{
 for (ui16 x = 0; x < SCREEN_DX; x++)
  {
   for (ui16 y = 0; y < SCREEN_DY / 2; y++)
    {
     Screen[x][y] = colorHL[y / 25];
    }
   ScreenChange[x] = 1;
  }
}
//---------------------------------------------------------------------------------------------------
void DisplayFill(ui8 color)
{
 DisplayFillRect(0, 0, SCREEN_DX - 1, SCREEN_DY - 1, color);
}
//---------------------------------------------------------------------------------------------------
void DisplayTest()
{
 DisplayRainbow();
 // Please do not call DisplayRedraw here
}
//---------------------------------------------------------------------------------------------------
extern const DevFont font32x25;
extern const DevFont font24x19;

/*
extern DevFont font24x18;
extern DevFont font24x19;
extern DevFont font27x21;
extern DevFont font28x21;
*/

static DevFont const *const Fonts[8] = {
 &font32x25,
 &font24x19,
 // &font21x15,
 // &font24x18,
 // &font24x19,
 // &font27x21,
 // &font28x21,
 0
};
//---------------------------------------------------------------------------------------------------
static const unsigned char *FindChr(const DevFont *f, unsigned int c)
{
 const ui32 numBlocks = sizeof(f->block) / sizeof(*(f->block));
 for (ui32 i = 0; i < numBlocks; ++i)
  {
   if ((c >= f->block[i].base) && (c <= f->block[i].base + f->block[i].sz))
    return &f->block[i].sym[(c - f->block[i].base) * (f->maxW * f->bytesH + 1)];
  }
 return f->block[0].sym;
}
//---------------------------------------------------------------------------------------------------
static unsigned int DisplayChr(unsigned int chr, unsigned int x, unsigned int y, const DevFont *f, ui8 color)
{
 color &= 0x07;
 const unsigned char *c = FindChr(f, chr);
 y += f->h;
 for (ui32 i = 0; (i < c[0]); ++i)
  {
   for (ui32 j = 0; (j < f->h); ++j)
    {
     if (c[i * f->bytesH + 1 + j / 8] & (1 << (j % 8)))
      DisplayPixel(x + i, y - j, color);
    }
  }
 return c[0];
}
//---------------------------------------------------------------------------------------------------
void DisplayText(const char *s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color)
{
 color &= 0x07;
 const DevFont *f = Fonts[fontType];
 if (f == 0)
  return;

 for (ui32 i = 0; s[i]; ++i)
  x += DisplayChr(s[i], x, y, f, color);
}
//---------------------------------------------------------------------------------------------------
void DisplayTextW(const WIDE_CHAR *s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color)
{
 color &= 0x07;
 const DevFont *f = Fonts[fontType];
 if (f == 0)
  return;

 for (ui32 i = 0; s[i]; ++i)
  x += DisplayChr(s[i], x, y, f, color);
}
//---------------------------------------------------------------------------------------------------
void CopyTileToScreen(const void *tile, int x, int y)
{
 for (int i = 0; i < TILE_DX; ++i)
  for (int j = 0; j < TILE_DY / 2; ++j)
   {
    int ii = i + x;
    int jj = j + y;
    if ((ii >= 0) && (ii < SCREEN_DX))
     {
      if ((jj >= 0) && (jj < SCREEN_DY / 2))
       {
        Screen[ii][SCREEN_DY / 2 - 1 - jj] = *((ui8 *)tile + i * TILE_DY / 2 + j);
        ScreenChange[ii] = 1;
       }
     }
   }
}
//---------------------------------------------------------------------------------------------------
