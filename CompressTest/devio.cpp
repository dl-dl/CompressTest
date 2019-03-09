#include "sizes.h"
#include "devio.h"
#include "devioimpl.h"

DeviceInput input[NUM_DEV];

void GetAdc(int id)
{
}

bool GpsReady(int id)
{
 return input[id].gps.size() > 0;
}

bool GetGps(PointFloat *dst, int id)
{
 *dst = input[id].gps.front();
 input[id].gps.pop_front();
 return true;
}

bool CompassReady(int id)
{
 return input[id].compass.size() > 0;
}

void GetCompass(CompassData *dst, int id)
{
 *dst = input[id].compass.front();
 input[id].compass.pop_front();
}

bool ButtonReady(int id)
{
 return input[id].button.size() > 0;
}

ui8 GetButton(int id)
{
 auto res = input[id].button.front();
 input[id].button.pop_front();
 return res;
}

bool RadioReady(int id)
{
 return input[id].radio.size() > 0;
}

void GetRadio(RadioMsg *dst, int id)
{
 *dst = input[id].radio.front();
 input[id].radio.pop_front();
}

bool UsbReady(int id)
{
 return false;
}

ui32 GetUsb(ui8 **buff, int id)
{
 return 0;
}

void Broadcast(int hardwareId, PointInt point, int id)
{
 RadioMsg msg;
 *(int *)(msg.data + 2) = point.x;
 *(int *)(msg.data + 6) = point.y;
 msg.data[0] = hardwareId;
 for (int i = 0; i < NUM_DEV; ++i)
  if (i != id)
   input[i].radio.push_back(msg);
}
