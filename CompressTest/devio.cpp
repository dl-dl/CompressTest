#include "types.h"
#include "devio.h"
#include "devioimpl.h"
#include "tourist.h"

DeviceInput input;
extern "C"
{
 ui32 CoordTileX = -1;
 ui32 CoordTileY = -1;
 si8 MapShiftH;
 si8 MapShiftV;
 si8 MapZoom = MIN_ZOOM_LEVEL;
 TTourist Tourist[10];
}

void GetAdc()
{
}

bool GpsReady()
{
 return input.gps.size() > 0;
}

bool GetGps(PointFloat *dst)
{
 *dst = input.gps.front();
 input.gps.pop_front();
 return true;
}

bool CompassReady()
{
 return input.compass.size() > 0;
}

void GetCompass(CompassData *dst)
{
 *dst = input.compass.front();
 input.compass.pop_front();
}
