#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "types.h"
#include "coord.h"
#include "sizes.h"
#include "fs.h"
#include "fscache.h"
#include "devio.h"

typedef struct
{
 ui32 hardwareId;
 ui32 x;
 ui32 y;
} GroupItem;

typedef struct
{
 GroupItem g[16];
 ui32 n;
} GroupData;

#ifdef __cplusplus
extern "C"
{
#endif
 void DeviceInit(void);
 void Run(void);
 void ScreenPaint(void);
 bool NeedRedraw(void);
 void ResetRedraw(void);
 void ProcessButton(void);
 void ProcessGps(void);
 void ProcessRadio(void);
 void ProcessCompass(void);
 void ProcessUsb(void);
#ifdef __cplusplus
}
#endif

#endif // !__DEVICE_H__
