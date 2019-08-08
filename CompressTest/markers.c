#include "graph.h"
#include "sizes.h"
#include "color.h"
#include "tourist.h"
#include "map.h"

extern ui32 CoordTileX;
extern ui32 CoordTileY;
extern si8 MapShiftH;
extern si8 MapShiftV;
extern si8 MapZoom;

int volatile DemoY;
ui8 DemoTouristOn;

/*
static void DrawHaircross(int x, int y, int dd)
{
 for (int k = -1; k <= 1; ++k)
  DisplayLine(x - dd, y + k, x + dd, y + k, (k == 0) ? CLR_RED : CLR_WHITE);
 for (int k = -1; k <= 1; ++k)
  DisplayLine(x + k, y - dd, x + k, y + dd, (k == 0) ? CLR_RED : CLR_WHITE);
}

static void DrawArrow(int x, int y)
{
 const int sz = 5;
 DisplayLine(x - sz, y - sz, x + sz, y + sz, CLR_RED);
 DisplayLine(x - sz, y + sz, x + sz, y - sz, CLR_RED);
}
*/
static void DrawTouristMarker(int x, int y, int id)
{
 if (x < 10)
  x = 10;
 else if (x >= SCREEN_DX - 11)
  x = SCREEN_DX - 11;
 if (y < 10)
  y = 10;
 else if (y > SCREEN_DY - 11)
  y = SCREEN_DY - 11;

 if (id == 1)
  {
   DisplayFillCircle(x, y, 14, clRed);
   DisplayCircle(x, y, 11, clYellow);
   DisplayCircle(x, y, 12, clYellow);
   DisplayText("1", x - 5, y - 12, 1, clYellow);
  }
 else
  {
   DisplayFillRomb(x, y, 34, clRed);
   DisplayRomb(x, y, 26, clYellow);
   DisplayRomb(x, y, 28, clYellow);
   DisplayText("2", x - 5, y - 12, 1, clYellow);
  }
}

void DrawGroup()
{
 const PointInt center = GetScreenCenter();
 for (ui32 i = 0; i < sizeof(Tourist) / sizeof(*Tourist); ++i)
  if (Tourist[i].IsRx)
   {
    const TTourist *p = Tourist + i;
    int x = ScaleDownCoord(p->CoordTileX, MapZoom) - ScaleDownCoord(center.x, MapZoom) + SCREEN_DX / 2;
    int y = ScaleDownCoord(p->CoordTileY, MapZoom) - ScaleDownCoord(center.y, MapZoom) + SCREEN_DY / 2;
    DrawTouristMarker(x, SCREEN_DY - y, i);
   }

 int x = ScaleDownCoord(CoordTileX, MapZoom) - ScaleDownCoord(center.x, MapZoom) + SCREEN_DX / 2;
 int y = ScaleDownCoord(CoordTileY, MapZoom) - ScaleDownCoord(center.y, MapZoom) + SCREEN_DY / 2;
 DrawTouristMarker(x, SCREEN_DY - y, 1);

 if (DemoTouristOn && DemoY >= 10)
  {
   y = SCREEN_DY - y + DemoY / 2;
   x = x + DemoY / 18;
   DisplayFillRomb(x, y, 34, clRed);
   DisplayRomb(x, y, 26, clYellow);
   DisplayRomb(x, y, 28, clYellow);
   DisplayText("2", x - 5, y - 12, 1, clYellow);
  }
}
