#include "sizes.h"
#include "devio.h"
#include "devioimpl.h"

DeviceInput input[NUM_DEV];

bool GpsReady(int id)
{
 return input[id].gps.size() > 0;
}

void GetGps(PointFloat *dst, int id)
{
 *dst = input[id].gps.front();
 input[id].gps.pop_front();
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

void GetButton(ui8 *dst, int id)
{
 *dst = input[id].button.front();
 input[id].button.pop_front();
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

void Broadcast(int hardwareId, PointInt point, int id)
{
 RadioMsg msg;
 msg.pos = point;
 msg.hardwareId = hardwareId;
 for (int i = 0; i < NUM_DEV; ++i)
  if (i != id)
   input[i].radio.push_back(msg);
}
