#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif
 void DeviceInit(void);
 void Run(void);
 void ScreenPaint(void);
 bool NeedRedraw(void);
 void ResetRedraw(void);
#ifdef __cplusplus
}
#endif

#endif // !__DEVICE_H__
