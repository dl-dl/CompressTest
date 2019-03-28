#include "sizes.h"
#include "devio.h"
#include "devioimpl.h"

DeviceInput input;
extern "C"
{
 ui32 CoordTileX = -1;
 ui32 CoordTileY = -1;
 ui8 MapZoom = 12;
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
