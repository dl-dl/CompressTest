#ifndef __TEXT_H__
#define __TEXT_H__

#include "types.h"
#include "screen.h"

//#define FONT_NORMAL 0

void PrintStr(const char* s, ui32 x, ui32 y, ui8 fontType, ui8 color, Screen* screen);
void PrintStrW(const ui16* s, ui32 x, ui32 y, ui8 fontType, ui8 color, Screen* screen);

#endif
