#ifndef _DEVIO_H
#define _DEVIO_H
#include "types.h"
#include "coord.h"

typedef struct
{
 int x, y, z;
} CompassData;

typedef struct
{
 PointInt pos;
 ui32 id;
} RadioMsg;

bool GpsReady(int id);
void GetGps(PointFloat *dst, int id);
bool CompassReady(int id);
void GetCompass(CompassData *dst, int id);
bool ButtonReady(int id);
void GetButton(ui8 *dst, int id);
bool RadioReady(int id);
void GetRadio(RadioMsg *dst, int id);
void Broadcast(int srcId, PointInt data);

#endif
