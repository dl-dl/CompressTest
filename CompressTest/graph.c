#include "types.h"
#include "graph.h"
#include "sizes.h"
#include <string.h>

Screen screen;

void DisplayClear()
{
 memset(&screen, 0, sizeof(screen));
}

void DisplayPixel(int x, int y, ui8 color)
{
 if ((unsigned)x >= SCREEN_CX)
  return;
 if ((unsigned)y >= SCREEN_CY)
  return;
 color &= 0x0F;
 ui8 b = screen.line[x].pix[y / 2];
 screen.line[x].pix[y / 2] = (y % 2) ? ((b & 0xF0) | color) : ((b & 0x0F) | (color << 4));
}

static void VLine(int x, int y, int height, ui8 color)
{
 ui8 *ptr = &screen.line[x].pix[y / 2];

 if (y % 2)
  {
   *ptr = (*ptr & 0xF0) | color;
   ptr++;
   height--;
  }

 for (ui16 cnt = height / 2; cnt--;)
  *ptr++ = (color << 4) | color;

 if (height % 2)
  *ptr = ((*ptr & 0x0F) | (color << 4));
}

void DisplayFillRect(int left, int top, int width, int height, ui8 color)
{
 if (height <= 0)
  return;
 if (width <= 0)
  return;
 if (left + width > SCREEN_CX)
  return;
 if (top + height > SCREEN_CY)
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
 for (int i = 0; i < TILE_CX; ++i)
  for (int j = 0; j < TILE_CY / 2; ++j)
   {
    int ii = i + x;
    int jj = j + y;
    if ((ii >= 0) && (ii < SCREEN_CX))
     if ((jj >= 0) && (jj < SCREEN_CY / 2))
      screen.line[ii].pix[jj] = *((ui8 *)tile + i * TILE_CY / 2 + j);
   }
}
