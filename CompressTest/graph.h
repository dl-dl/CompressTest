#ifndef __GRAPH_H__
#define __GRAPH_H__
#include "types.h"
#include "screen.h"

#ifdef __cplusplus
extern "C"
{
#endif
 void DisplayInit(void);
 void DisplayClear(void);
 void DisplayPixel(int x, int y, ui8 color);
 void DisplayLine(int x0, int y0, int x1, int y1, ui8 color);
 void DisplayFillRect(int left, int top, int width, int height, ui8 color);
 void DisplayCircle(int x, int y, int r, ui8 color);
 void CopyTileToScreen(const void *tile, int x, int y);
#ifdef __cplusplus
}
#endif

#endif