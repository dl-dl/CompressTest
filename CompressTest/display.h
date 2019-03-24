#ifndef __PAINT_H
#define __PAINT_H

#include "types.h"

#ifdef _WINDOWS
void DisplayRedraw(HDC hdc);
ui8 TestButton(int x, int y);
#endif
#endif
