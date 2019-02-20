#ifndef __PAINT_H
#define __PAINT_H

#include "types.h"
#include "screen.h"

void CopyScreen(HDC hdc, const Screen* screen);
ui8 TestButton(int x, int y);

#endif
