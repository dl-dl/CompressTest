#ifndef __TEXT_H__
#define __TEXT_H__

#include "types.h"
#include "screen.h"

//#define FONT_NORMAL 0
#ifdef __cplusplus
extern "C"
{
#endif
 void DisplayText(const char *s, ui32 x, ui32 y, ui8 fontType, ui8 color);
 void DisplayTextW(const ui16 *s, ui32 x, ui32 y, ui8 fontType, ui8 color);
#ifdef __cplusplus
}
#endif

#endif
