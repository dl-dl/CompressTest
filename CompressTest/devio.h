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
 ui8 data[10];
} RadioMsg;

#ifdef __cplusplus
extern "C"
{
#endif
 void GetAdc(int id);
 bool GpsReady(int id);
 bool GetGps(PointFloat *dst, int id);
 bool CompassReady(int id);
 void GetCompass(CompassData *dst, int id);
 bool ButtonReady(int id);
 ui8 GetButton(int id);
 bool RadioReady(int id);
 void GetRadio(RadioMsg *dst, int id);
 bool UsbReady(int id);
 ui32 GetUsb(ui8** buff, int id);
 void Broadcast(int hardwareId, PointInt data, int id);
#ifdef __cplusplus
}
#endif
#endif
