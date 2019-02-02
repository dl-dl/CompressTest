#ifndef __GRAPH_H__
#define __GRAPH_H__
#include "types.h"
#include "screen.h"

void Pixel(int x, int y, ui8 color, Screen* screen);
void Line(int x0, int y0, int x1, int y1, ui8 color, Screen* screen);
void FillRect(int left, int top, int width, int height, ui8 color, Screen* screen);
void Circle(int x, int y, int r, ui8 color, Screen* screen);
void CopyTileToScreen(const void* tile, int x, int y, Screen* screen);

#endif