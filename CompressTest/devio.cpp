#include "sizes.h"
#include "devio.h"
#include "devioimpl.h"

DeviceInput input;

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

bool ButtonReady()
{
 return input.button.size() > 0;
}

ui8 GetButton()
{
 auto res = input.button.front();
 input.button.pop_front();
 return res;
}

bool RadioReady()
{
 return input.radio.size() > 0;
}

void GetRadio(RadioMsg *dst)
{
 *dst = input.radio.front();
 input.radio.pop_front();
}

bool UsbReady()
{
 return false;
}

ui32 GetUsb(ui8 **buff)
{
 return 0;
}

void Broadcast(int hardwareId, PointInt point)
{
 RadioMsg msg;
 *(int *)(msg.data + 2) = point.x;
 *(int *)(msg.data + 6) = point.y;
 msg.data[0] = hardwareId;
}
