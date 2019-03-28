#ifndef __GRAPH_H__
#define __GRAPH_H__
#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif
 void DisplayClear(ui8 color);
 void DisplayPixel(int x, int y, ui8 color);
 void DisplayLine(int x0, int y0, int x1, int y1, ui8 color);
 void DisplayFillRect(int left, int top, int width, int height, ui8 color);
 void DisplayCircle(int x, int y, int r, ui8 color);
 void CopyTileToScreen(const void *tile, int x, int y);
 void DisplayRainbow(void);

 void DisplayText(const char *s, ui32 x, ui32 y, ui8 fontType, ui8 color);
 void DisplayTextW(const ui16 *s, ui32 x, ui32 y, ui8 fontType, ui8 color);
#ifdef __cplusplus
}
#endif

#endif