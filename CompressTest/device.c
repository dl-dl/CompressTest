#include "device.h"
#include "fs.h"
#include "sd.h"
#include "coord.h"
#include "convert.h"
#include "graph.h"
#include "sound.h"
#include "devio.h"
#include "color.h"
#if CREATE_NEW_SD
#include "fsinit.h"
#endif

#include <string.h>
#include <math.h>

typedef struct
{
 FsMapCache FsCache;
 PointInt currentPos;
 PointInt screenStart;
 ui8 zoom;
 GroupData group;
 CompassData currentCompass;

 int hardwareId;
 bool redrawScreen;
} Device;

Device dev;

static void AdjustScreenPos(PointInt pos);
static void DrawMap(void);
static void DrawGroup(void);
static void DrawCompass(void);

void DeviceInit()
{
#if CREATE_NEW_SD
 FsFormat();
 FsInit();
#endif
 //	BlockAddr sz = FsFreeSpace();
 CacheInit(&dev.FsCache);

 dev.group.n = 0;
 dev.hardwareId = 3001;
 dev.zoom = 12;
 dev.currentPos.y = dev.currentPos.x = 0x7FFFFFFF;
}

static inline int intabs(int a)
{
 return (a >= 0) ? a : -a;
}

void AdjustScreenPos(PointInt pos)
{
 pos.x = ScaleDownCoord(pos.x, dev.zoom);
 pos.y = ScaleDownCoord(pos.y, dev.zoom);
 pos.x -= SCREEN_DX / 2;
 pos.y -= SCREEN_DY / 2;

 if ((intabs(dev.screenStart.x - pos.x) > SCREEN_DX / 8) ||
     (intabs(dev.screenStart.y - pos.y) > SCREEN_DY / 8))
  {
   dev.screenStart = pos;
  }
}

void DrawMap()
{
 AdjustScreenPos(dev.currentPos);
#ifdef _DEBUG
 DisplayClear(0);
#endif
 if (!CacheFetchIMS(&dev.FsCache, dev.currentPos.x / TILE_DX, dev.currentPos.y / TILE_DY))
  return;

 for (int x = (dev.screenStart.x / TILE_DX) * TILE_DX; x < dev.screenStart.x + SCREEN_DX; x += TILE_DX)
  for (int y = (dev.screenStart.y / TILE_DY) * TILE_DY; y < dev.screenStart.y + SCREEN_DY; y += TILE_DY)
   {
    ui32 index = CacheRead(&dev.FsCache, x / TILE_DX, y / TILE_DY, dev.zoom);
    CopyTileToScreen(dev.FsCache.map[index].data, x - dev.screenStart.x, (y - dev.screenStart.y) / 2);
   }
}

static void DrawTouristMarker(ui32 x, ui32 y, int dd, int hardwareId)
{
 for (int k = -1; k <= 1; ++k)
  DisplayLine(x - dd, y + k, x + dd, y + k, (k == 0) ? CLR_RED : CLR_WHITE);
 for (int k = -1; k <= 1; ++k)
  DisplayLine(x + k, y - dd, x + k, y + dd, (k == 0) ? CLR_RED : CLR_WHITE);
 char s[2];
 s[0] = '0' + hardwareId % 8;
 s[1] = 0;
 DisplayText(s, x + 2, y, 0, CLR_RED);
}

void DrawGroup()
{
 for (ui32 i = 0; i < dev.group.n; ++i)
  {
   const GroupItem *p = &dev.group.g[i];
   ui32 x = ScaleDownCoord(p->x, dev.zoom) - dev.screenStart.x;
   ui32 y = ScaleDownCoord(p->y, dev.zoom) - dev.screenStart.y;
   DrawTouristMarker(x, SCREEN_DY - y, 5, p->hardwareId);
  }
 ui32 x = ScaleDownCoord(dev.currentPos.x, dev.zoom) - dev.screenStart.x;
 ui32 y = ScaleDownCoord(dev.currentPos.y, dev.zoom) - dev.screenStart.y;
 DrawTouristMarker(x, SCREEN_DY - y, 10, dev.hardwareId);
}

void DrawCompass()
{
 const int COMPASS_POS_X = 20;
 const int COMPASS_POS_Y = SCREEN_DY - 20;
 const int COMPASS_R = 16;

 DisplayCircle(COMPASS_POS_X, COMPASS_POS_Y, COMPASS_R, CLR_RED);
 const ui32 MIN2_COMPASS = 0x100;  // TODO: find reasonable value
 const ui32 MIN3_COMPASS = 0x1000; // TODO: find reasonable value
 CompassData d = dev.currentCompass;
 ui32 r2 = d.x * d.x + d.y * d.y;
 ui32 r3 = r2 + d.z * d.z;
 if (r3 > MIN3_COMPASS && r2 > MIN2_COMPASS)
  {
   float q = (float)COMPASS_R / sqrtf((float)r2);
   d.x = (ui32)(d.x * q);
   d.y = (ui32)(d.y * q);
  }
 else
  {
   return;
  }

 DisplayLine(COMPASS_POS_X - d.y, COMPASS_POS_Y + d.x, COMPASS_POS_X + d.y, COMPASS_POS_Y - d.x, CLR_RED);
 DisplayCircle(COMPASS_POS_X - d.y, COMPASS_POS_Y + d.x, COMPASS_R / 8, CLR_RED);
 DisplayCircle(COMPASS_POS_X - d.y, COMPASS_POS_Y + d.x, COMPASS_R / 8 + 1, CLR_WHITE);
}

static int FindInGroup(const GroupData *g, int hardwareId)
{
 for (ui32 i = 0; i < g->n; ++i)
  {
   if (g->g[i].hardwareId == hardwareId)
    return i;
  }
 return -1;
}

void ProcessGps()
{
 PointFloat fpoint;
 if (!GetGps(&fpoint))
  return;

 PointInt pos;
 pos.x = lon2tilex(fpoint.x, MAX_ZOOM_LEVEL);
 pos.y = lat2tiley(fpoint.y, MAX_ZOOM_LEVEL);
 if (PointEqInt(&pos, &dev.currentPos))
  return;
 dev.currentPos = pos;
 dev.redrawScreen = true;
}

void ProcessRadio()
{
 RadioMsg msg;
 GetRadio(&msg);

 int i = FindInGroup(&dev.group, msg.data[0]);
 if (i >= 0)
  {
   // assert( dev.group.g[i].hardwareId == msg.data[0]);
   dev.group.g[i].x = *(int *)&msg.data[2];
   dev.group.g[i].y = *(int *)&msg.data[6];
  }
 else
  {
   int n = dev.group.n++;
   dev.group.g[n].hardwareId = msg.data[0];
   dev.group.g[n].x = *(int *)&msg.data[2];
   dev.group.g[n].y = *(int *)&msg.data[6];
  }
 dev.redrawScreen = true;
}

void ProcessCompass()
{
 CompassData d;
 GetCompass(&d);

 dev.currentCompass = d;
 dev.redrawScreen = true;
}

void ProcessButton()
{
 ui8 b = GetButton();
 if (b == 0x20 && dev.zoom < 15)
  dev.zoom++;
 else if (b == 0x80 && dev.zoom > 12)
  dev.zoom--;
 else
  return;
 Sound(1, 100);
 dev.screenStart.x = dev.screenStart.y = 0x7FFFFFFF;
 dev.redrawScreen = true;
}

void ProcessUsb()
{
 ui8 *data;
 ui32 sz = GetUsb(&data);
 ui32 sectorNum = *((ui32 *)data);
 data += 4;
 sz -= 4;
 int n = sz / BLOCK_SIZE;
 if (sz % BLOCK_SIZE)
  n++;
 SDCardWrite(sectorNum, data, 1);
}

void ScreenPaint()
{
 DrawMap();
 DrawCompass();
 DrawGroup();
}

void Run()
{
 // GetAdc();
 /*
 while (UsbReady())
  {
   ProcessUsb();
  }
*/
 while (GpsReady())
  {
   ProcessGps();
  }
 while (RadioReady())
  {
   ProcessRadio();
  }
 while (CompassReady())
  {
   ProcessCompass();
  }
 while (ButtonReady())
  {
   ProcessButton();
  }
}

bool NeedRedraw()
{
 return dev.redrawScreen;
}

void ResetRedraw()
{
 dev.redrawScreen = false;
}
