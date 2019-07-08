#include "device.h"
#include "fileio.h"
#include "coord.h"
#include "convert.h"
#include "graph.h"
#include "sound.h"
#include "devio.h"
#include "color.h"
#include "map.h"
#include "fs.h"

#include "ff.h"

#include <string.h>
#include <math.h>

extern ui32 CoordTileX;
extern ui32 CoordTileY;

typedef struct
{
 CompassData currentCompass;
 int hardwareId;
 bool redrawScreen;
} Device;

static Device dev;
//#define FORAMT_FAT32
static void MountFat32()
{
 const TCHAR path[] = "0:/";
#ifdef FORAMT_FAT32
 static BYTE buff[512 * 32];
 f_mkfs(path, FM_FAT32, 512, &buff, sizeof(buff));
#endif
 FATFS fs;
 f_mount(&fs, path, 1);
}

void DeviceInit()
{
 MountFat32();
 MapInit();

 dev.hardwareId = 3001;
}
/*
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
*/
void ProcessGps()
{
 PointFloat fpoint;
 if (!GetGps(&fpoint))
  return;

 PointInt pos;
 pos.x = lon2tilex(fpoint.x, MAX_ZOOM_LEVEL);
 pos.y = lat2tiley(fpoint.y, MAX_ZOOM_LEVEL);
 CoordTileX = pos.x;
 CoordTileY = pos.y;
 dev.redrawScreen = true;
}

void ScreenPaint()
{
 DrawMap();
 DrawGroup();
}

void Run()
{
 while (GpsReady())
  {
   ProcessGps();
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
