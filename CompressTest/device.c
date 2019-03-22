#include "device.h"
#include "fs.h"
#include "sd.h"
#include "coord.h"
#include "convert.h"
#include "graph.h"
#include "text.h"
#include "devio.h"
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
 FsFormat(id);
 FsInit(id);
#endif
 //	BlockAddr sz = FsFreeSpace();
 CacheInit(&dev.FsCache);

 dev.group.n = 0;
 dev.hardwareId = 3000;
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
 pos.x -= SCREEN_CX / 2;
 pos.y -= SCREEN_CY / 2;

 if ((intabs(dev.screenStart.x - pos.x) > SCREEN_CX / 8) ||
     (intabs(dev.screenStart.y - pos.y) > SCREEN_CY / 8))
  {
   dev.screenStart = pos;
  }
}

void DrawMap()
{
 AdjustScreenPos(dev.currentPos);
#ifdef _DEBUG
 DisplayClear();
#endif
 PointInt pos;
 pos.x = dev.currentPos.x / TILE_CX;
 pos.y = dev.currentPos.y / TILE_CY;
 if (!CacheFetchIMS(&dev.FsCache, pos.x, pos.y))
  return;

 for (int x = (dev.screenStart.x / TILE_CX) * TILE_CX; x < dev.screenStart.x + SCREEN_CX; x += TILE_CX)
  for (int y = (dev.screenStart.y / TILE_CY) * TILE_CY; y < dev.screenStart.y + SCREEN_CY; y += TILE_CY)
   {
    ui32 index = CacheRead(&dev.FsCache, x / TILE_CX, y / TILE_CY, dev.zoom);
    CopyTileToScreen(dev.FsCache.map[index].data, x - dev.screenStart.x, (y - dev.screenStart.y) / 2);
   }
}

static void DrawTouristMarker(ui32 x, ui32 y, int dd, int hardwareId)
{
 for (int k = -1; k <= 1; ++k)
  DisplayLine(x - dd, y + k, x + dd, y + k, (k == 0) ? DEV_RED : DEV_WHITE);
 for (int k = -1; k <= 1; ++k)
  DisplayLine(x + k, y - dd, x + k, y + dd, (k == 0) ? DEV_RED : DEV_WHITE);
 char s[2];
 s[0] = '0' + hardwareId % 8;
 s[1] = 0;
 DisplayText(s, x + 2, y - 27, 0, DEV_RED);
}

void DrawGroup()
{
 for (ui32 i = 0; i < dev.group.n; ++i)
  {
   const GroupItem *p = &dev.group.g[i];
   ui32 x = ScaleDownCoord(p->x, dev.zoom) - dev.screenStart.x;
   ui32 y = ScaleDownCoord(p->y, dev.zoom) - dev.screenStart.y;
   DrawTouristMarker(x, y, 5, p->hardwareId);
  }
 ui32 x = ScaleDownCoord(dev.currentPos.x, dev.zoom) - dev.screenStart.x;
 ui32 y = ScaleDownCoord(dev.currentPos.y, dev.zoom) - dev.screenStart.y;
 DrawTouristMarker(x, y, 10, dev.hardwareId);
}

void DrawCompass()
{
 const int COMPASS_POS_X = 20;
 const int COMPASS_POS_Y = 20;
 const int COMPASS_R = 16;
 //	const int COMPASS_POS_X = 120;
 //	const int COMPASS_POS_Y = 200;
 //	const int COMPASS_R = COMPASS_POS_X - 8;

 DisplayCircle(COMPASS_POS_X, COMPASS_POS_Y, COMPASS_R, DEV_RED);
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

 DisplayLine(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y, COMPASS_POS_X + d.x, COMPASS_POS_Y + d.y, DEV_RED);
 DisplayCircle(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y, COMPASS_R / 8, DEV_RED);
 DisplayCircle(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y, COMPASS_R / 8 + 1, DEV_RED | DEV_GREEN | DEV_BLUE);
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
 Broadcast(dev.hardwareId, pos);
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
