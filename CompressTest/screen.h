#ifndef __SCREEN_H
#define __SCREEN_H
#include "types.h"
#include "sizes.h"

typedef struct
{
	ui8 pix[SCREEN_DY / 2];
} ScreenLine;

#ifdef __cplusplus
extern "C"
{
#endif
 extern ScreenLine screen[SCREEN_DX];
#ifdef __cplusplus
}
#endif

#endif
