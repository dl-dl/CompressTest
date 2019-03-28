#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "types.h"

#ifdef _WINDOWS
void DisplayInit(void);
void DisplayRedraw(HDC hdc);
ui8 TestButton(int x, int y);
#endif
#endif
