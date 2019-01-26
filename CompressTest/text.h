#ifndef __TEXT_H__
#define __TEXT_H__

#include "types.h"
#include "screen.h"

//#define FONT_NORMAL 0

typedef unsigned short WIDE_CHAR;

void PrintStr(const char* s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color, Screen* screen);
void PrintStrW(const WIDE_CHAR* s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color, Screen* screen);

#endif
