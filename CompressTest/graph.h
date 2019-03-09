#ifndef __GRAPH_H__
#define __GRAPH_H__
#include "types.h"
#include "screen.h"

#ifdef __cplusplus
extern "C"
{
#endif
 void DisplayClear(Screen *screen);
 void DisplayPixel(int x, int y, ui8 color, Screen *screen);
 void DisplayLine(int x0, int y0, int x1, int y1, ui8 color, Screen *screen);
 void DisplayFillRect(int left, int top, int width, int height, ui8 color, Screen *screen);
 void DisplayCircle(int x, int y, int r, ui8 color, Screen *screen);
 void CopyTileToScreen(const void *tile, int x, int y, Screen *screen);
#ifdef __cplusplus
}
#endif

#endif