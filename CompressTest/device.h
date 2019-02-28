#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "types.h"
#include "coord.h"
#include "sizes.h"
#include "fs.h"
#include "fscache.h"
#include "screen.h"
#include "devio.h"

typedef struct
{
 RadioMsg data[16];
 ui32 n;
} GroupData;

#ifdef __cplusplus
extern "C"
{
#endif
 void Init(int id);
 void Run(int id);
 void Paint(int id);
 bool NeedRedraw(int id);
 void ResetRedraw(int id);
 Screen *GetScreen(int id);
 void ProcessButton(ui8 b, int id);
 void ProcessGps(PointFloat point, int id);
 void ProcessRadio(const RadioMsg *point, int id);
 void ProcessCompass(CompassData d, int id);
#ifdef __cplusplus
}
#endif

#endif // !__DEVICE_H__
