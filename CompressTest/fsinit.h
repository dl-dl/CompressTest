#ifndef __FSINIT_H
#define __FSINIT_H

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif
 void FsInit(void);
 void FsFormat(void);
 ui32 FsFreeSpace(void);
#ifdef __cplusplus
}
#endif

#endif