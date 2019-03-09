#include "device.h"
#include "fs.h"
#include "sd.h"
#include "coord.h"
#include "convert.h"
#include "graph.h"
#include "text.h"
#include "devio.h"

#include <string.h>
#include <math.h>
#include <assert.h>

typedef struct
{
 FsMapCache FsCache;
 PointInt currentPos;
 PointInt screenStart;
 ui8 zoom;
 GroupData group;
 CompassData currentCompass;

 int hardwareId;
 Screen screen;
 bool redrawScreen;
} Device;

Device dev[NUM_DEV];

static void AdjustScreenPos(PointInt pos, int id);
static void DrawMap(int id);
static void DrawGroup(int id);
static void DrawCompass(int id);

void DeviceInit(int id)
{
#if CREATE_NEW_SD
 FsFormat(id);
 FsInit(id);
#endif
 //	BlockAddr sz = FsFreeSpace(id);
 CacheInit(&dev[id].FsCache);

 dev[id].group.n = 0;
 dev[id].hardwareId = 3000 + id;
 dev[id].zoom = 12;
 dev[id].currentPos.y = dev[id].currentPos.x = 0x7FFFFFFF;
}

static inline int intabs(int a)
{
 return (a >= 0) ? a : -a;
}

void AdjustScreenPos(PointInt pos, int id)
{
 pos.x = ScaleDownCoord(pos.x, dev[id].zoom);
 pos.y = ScaleDownCoord(pos.y, dev[id].zoom);
 pos.x -= SCREEN_CX / 2;
 pos.y -= SCREEN_CY / 2;

 if ((intabs(dev[id].screenStart.x - pos.x) > SCREEN_CX / 8) ||
     (intabs(dev[id].screenStart.y - pos.y) > SCREEN_CY / 8))
  {
   dev[id].screenStart = pos;
  }
}

void DrawMap(int id)
{
#ifdef _DEBUG
 DisplayClear(&dev[id].screen);
#endif
 PointInt pos;
 pos.x = dev[id].currentPos.x / TILE_CX;
 pos.y = dev[id].currentPos.y / TILE_CY;
 if (!CacheFetchIMS(&dev[id].FsCache, pos.x, pos.y, id))
  return;

 for (int x = (dev[id].screenStart.x / TILE_CX) * TILE_CX; x < dev[id].screenStart.x + SCREEN_CX; x += TILE_CX)
  for (int y = (dev[id].screenStart.y / TILE_CY) * TILE_CY; y < dev[id].screenStart.y + SCREEN_CY; y += TILE_CY)
   {
    ui32 index = CacheRead(&dev[id].FsCache, x / TILE_CX, y / TILE_CY, dev[id].zoom, id);
    CopyTileToScreen(dev[id].FsCache.map[index].data, x - dev[id].screenStart.x, (y - dev[id].screenStart.y) / 2, &dev[id].screen);
   }
}

void DrawGroup(int id)
{
 for (ui32 i = 0; i < dev[id].group.n; ++i)
  {
   PointInt pos;
   pos.x = *(int *)(dev[id].group.data[i].data + 2);
   pos.y = *(int *)(dev[id].group.data[i].data + 6);
   int x = ScaleDownCoord(pos.x, dev[id].zoom) - dev[id].screenStart.x;
   int y = ScaleDownCoord(pos.y, dev[id].zoom) - dev[id].screenStart.y;
   int dd = (dev[id].group.data[i].data[0] == dev[id].hardwareId) ? 10 : 5;
   DisplayLine(x - dd, y, x + dd, y, DEV_RED, &dev[id].screen);
   DisplayLine(x, y - dd, x, y + dd, DEV_RED, &dev[id].screen);
   char s[2];
   s[0] = '0' + dev[id].group.data[i].data[0] % 10;
   s[1] = 0;
   DisplayText(s, x + 2, y - 27, 0, DEV_RED, &dev[id].screen);
  }
}

void DrawCompass(int id)
{
 const int COMPASS_POS_X = 20;
 const int COMPASS_POS_Y = 20;
 const int COMPASS_R = 16;
 //	const int COMPASS_POS_X = 120;
 //	const int COMPASS_POS_Y = 200;
 //	const int COMPASS_R = COMPASS_POS_X - 8;

 DisplayCircle(COMPASS_POS_X, COMPASS_POS_Y, COMPASS_R, DEV_RED, &dev[id].screen);
 const ui32 MIN2_COMPASS = 0x100;  // TODO: find reasonable value
 const ui32 MIN3_COMPASS = 0x1000; // TODO: find reasonable value
 CompassData d = dev[id].currentCompass;
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

 DisplayLine(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y,
             COMPASS_POS_X + d.x, COMPASS_POS_Y + d.y, DEV_RED, &dev[id].screen);
 DisplayCircle(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y, COMPASS_R / 8, DEV_RED, &dev[id].screen);
 DisplayCircle(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y, COMPASS_R / 8 + 1, DEV_RED | DEV_GREEN | DEV_BLUE, &dev[id].screen);
}

static int FindInGroup(const GroupData *g, int hardwareId)
{
 for (ui32 i = 0; i < g->n; ++i)
  {
   if (g->data[i].data[0] == hardwareId)
    return i;
  }
 return -1;
}

void ProcessGps(int id)
{
 PointFloat fpoint;
 if (!GetGps(&fpoint, id))
  return;

 PointInt pos;
 pos.x = lon2tilex(fpoint.x, MAX_ZOOM_LEVEL);
 pos.y = lat2tiley(fpoint.y, MAX_ZOOM_LEVEL);
 if (PointEqInt(&pos, &dev[id].currentPos))
  return;
 dev[id].currentPos = pos;
 int i = FindInGroup(&dev[id].group, dev[id].hardwareId);
 if (i >= 0)
  {
   *(int *)(dev[id].group.data[i].data + 2) = pos.x;
   *(int *)(dev[id].group.data[i].data + 6) = pos.x;
  }
 else
  {
   i = dev[id].group.n;
   dev[id].group.data[i].data[0] = dev[id].hardwareId;
   *(int *)(dev[id].group.data[i].data + 2) = pos.x;
   *(int *)(dev[id].group.data[i].data + 6) = pos.x;
   dev[id].group.n++;
  }
 Broadcast(dev[id].hardwareId, pos, id);
 dev[id].redrawScreen = true;
}

void ProcessRadio(int id)
{
 RadioMsg msg;
 GetRadio(&msg, id);

 int i = FindInGroup(&dev[id].group, msg.data[0]);
 if (i >= 0)
  dev[id].group.data[i] = msg;
 else
  dev[id].group.data[dev[id].group.n++] = msg;
 dev[id].redrawScreen = true;
}

void ProcessCompass(int id)
{
 CompassData d;
 GetCompass(&d, id);

 dev[id].currentCompass = d;
 dev[id].redrawScreen = true;
}

void ProcessButton(int id)
{
 ui8 b = GetButton(id);
 if (b == 2 && dev[id].zoom < 14)
  dev[id].zoom++;
 else if (b == 1 && dev[id].zoom > 12)
  dev[id].zoom--;

 dev[id].screenStart.x = dev[id].screenStart.y = 0x7FFFFFFF;
 dev[id].redrawScreen = true;
}

void ProcessUsb(int id)
{
 ui8 *data;
 ui32 sz = GetUsb(&data, id);
 ui32 sectorNum = *((ui32 *)data);
 data += 4;
 sz -= 4;
 int n = sz / BLOCK_SIZE;
 if (sz % BLOCK_SIZE)
  n++;

 while (n--)
  {
   SDCardWrite(sectorNum, data, id);
   sectorNum++;
   data += BLOCK_SIZE;
  }
}

void ScreenPaint(int id)
{
 AdjustScreenPos(dev[id].currentPos, id);
 DrawMap(id);
 DrawCompass(id);
 DrawGroup(id);
}

void Run(int id)
{
 GetAdc(id);

 while (UsbReady(id))
  {
   ProcessUsb(id);
  }
 while (GpsReady(id))
  {
   ProcessGps(id);
  }
 while (RadioReady(id))
  {
   ProcessRadio(id);
  }
 while (CompassReady(id))
  {
   ProcessCompass(id);
  }
 while (ButtonReady(id))
  {
   ProcessButton(id);
  }
}

bool NeedRedraw(int id)
{
 return dev[id].redrawScreen;
}

void ResetRedraw(int id)
{
 dev[id].redrawScreen = false;
}

Screen *GetScreen(int id)
{
 return &dev[id].screen;
}
