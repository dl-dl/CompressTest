#ifndef __SCREEN_H
#define __SCREEN_H
#include "types.h"
#include "sizes.h"

typedef struct
{
	ui8 pix[SCREEN_CY / 2];
} ScreenLine;

typedef struct
{
	ScreenLine line[SCREEN_CX];
} Screen;

#ifdef __cplusplus
extern "C"
{
#endif
 extern Screen screen;
#ifdef __cplusplus
}
#endif

#endif
