#ifndef __GRAPH_H__
#define __GRAPH_H__
#include "types.h"
#include "screen.h"

void Pixel(ui32 x, ui32 y, ui8 color, Screen* screen);
void Line(ui32 x0, ui32 y0, ui32 x1, ui32 y1, ui8 color, Screen* screen);
void FillRect(ui32 left, ui32 top, int width, int height, ui8 color, Screen* screen);
void Circle(ui32 x, ui32 y, int r, ui8 color, Screen* screen);
void CopyTileToScreen(const void* tile, ui32 x, ui32 y, Screen* screen);

#endif