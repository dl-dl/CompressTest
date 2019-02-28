#include "device.h"
#include "fs.h"
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

void Init(int id)
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

void AdjustScreenPos(PointInt pos, int id)
{
 pos.x = ScaleDownCoord(pos.x, dev[id].zoom);
 pos.y = ScaleDownCoord(pos.y, dev[id].zoom);
 pos.x -= SCREEN_CX / 2;
 pos.y -= SCREEN_CY / 2;

 if ((abs(dev[id].screenStart.x - pos.x) > SCREEN_CX / 8) ||
     (abs(dev[id].screenStart.y - pos.y) > SCREEN_CY / 8))
  {
   dev[id].screenStart = pos;
  }
}

void DrawMap(int id)
{
#ifdef _DEBUG
 memset(&dev[id].screen, 0, sizeof(dev[0].screen));
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
   PointInt pos = dev[id].group.data[i].pos;
   int x = ScaleDownCoord(pos.x, dev[id].zoom) - dev[id].screenStart.x;
   int y = ScaleDownCoord(pos.y, dev[id].zoom) - dev[id].screenStart.y;
   int dd = (dev[id].group.data[i].hardwareId == dev[id].hardwareId) ? 10 : 5;
   Line(x - dd, y, x + dd, y, DEV_RED, &dev[id].screen);
   Line(x, y - dd, x, y + dd, DEV_RED, &dev[id].screen);
   char s[2];
   s[0] = '0' + dev[id].group.data[i].hardwareId % 10;
   s[1] = 0;
   PrintStr(s, x + 2, y - 27, 0, DEV_RED, &dev[id].screen);
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

 Circle(COMPASS_POS_X, COMPASS_POS_Y, COMPASS_R, DEV_RED, &dev[id].screen);
 const ui32 MIN2_COMPASS = 0x800;   // TODO: find reasonable value
 const ui32 MIN3_COMPASS = 0x10000; // TODO: find reasonable value
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

 Line(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y,
      COMPASS_POS_X + d.x, COMPASS_POS_Y + d.y, DEV_RED, &dev[id].screen);
 Circle(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y, COMPASS_R / 8, DEV_RED, &dev[id].screen);
 Circle(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y, COMPASS_R / 8 + 1, DEV_RED | DEV_GREEN | DEV_BLUE, &dev[id].screen);
}

static int FindInGroup(const GroupData *g, int hardwareId)
{
 for (ui32 i = 0; i < g->n; ++i)
  {
   if (g->data[i].hardwareId == hardwareId)
    return i;
  }
 return -1;
}

void ProcessGps(PointFloat fpoint, int id)
{
 PointInt pos;
 pos.x = lon2tilex(fpoint.x, MAX_ZOOM_LEVEL);
 pos.y = lat2tiley(fpoint.y, MAX_ZOOM_LEVEL);
 if (PointEqInt(&pos, &dev[id].currentPos))
  return;
 dev[id].currentPos = pos;
 int i = FindInGroup(&dev[id].group, dev[id].hardwareId);
 if (i >= 0)
  {
   dev[id].group.data[i].pos = pos;
  }
 else
  {
   i = dev[id].group.n;
   dev[id].group.data[i].hardwareId = dev[id].hardwareId;
   dev[id].group.data[i].pos = pos;
   dev[id].group.n++;
  }
 Broadcast(dev[id].hardwareId, pos, id);
 dev[id].redrawScreen = true;
}

void ProcessRadio(const RadioMsg *msg, int id)
{
 int i = FindInGroup(&dev[id].group, msg->hardwareId);
 if (i >= 0)
  dev[id].group.data[i] = *msg;
 else
  dev[id].group.data[dev[id].group.n++] = *msg;
 dev[id].redrawScreen = true;
}

void ProcessCompass(CompassData d, int id)
{
 dev[id].currentCompass = d;
 dev[id].redrawScreen = true;
}

void ProcessButton(ui8 b, int id)
{
 if (b == 2 && dev[id].zoom < 14)
  dev[id].zoom++;
 else if (b == 1 && dev[id].zoom > 12)
  dev[id].zoom--;

 dev[id].screenStart.x = dev[id].screenStart.y = 0x7FFFFFFF;
 dev[id].redrawScreen = true;
}

void Paint(int id)
{
 AdjustScreenPos(dev[id].currentPos, id);
 DrawMap(id);
 DrawCompass(id);
 DrawGroup(id);
}

void Run(int id)
{
 while (GpsReady(id))
  {
   PointFloat d;
   GetGps(&d, id);
   ProcessGps(d, id);
  }
 while (RadioReady(id))
  {
   RadioMsg d;
   GetRadio(&d, id);
   ProcessRadio(&d, id);
  }
 while (CompassReady(id))
  {
   CompassData d;
   GetCompass(&d, id);
   ProcessCompass(d, id);
  }
 while (ButtonReady(id))
  {
   ui8 d;
   GetButton(&d, id);
   ProcessButton(d, id);
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
