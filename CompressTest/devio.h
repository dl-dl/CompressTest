#ifndef _DEVIO_H
#define _DEVIO_H
#include "types.h"
#include "coord.h"

typedef struct
{
 int x, y, z;
} CompassData;

#ifdef __cplusplus
extern "C"
{
#endif
 bool GpsReady(void);
 bool GetGps(PointFloat *dst);
#ifdef __cplusplus
}
#endif
#endif
