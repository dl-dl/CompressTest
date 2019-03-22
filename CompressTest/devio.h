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
 void GetAdc(void);
 bool GpsReady(void);
 bool GetGps(PointFloat *dst);
 bool CompassReady(void);
 void GetCompass(CompassData *dst);
 bool ButtonReady(void);
 ui8 GetButton(void);
 bool RadioReady(void);
 void GetRadio(RadioMsg *dst);
 bool UsbReady(void);
 ui32 GetUsb(ui8 **buff);
 void Broadcast(int hardwareId, PointInt data);
#ifdef __cplusplus
}
#endif
#endif
