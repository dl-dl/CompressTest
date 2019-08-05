#ifndef __GRAPH_H__
#define __GRAPH_H__
	
#include "types.h"
#include "color.h"

void DisplayFill(ui8 color);
void DisplayPixel(ui16 x, ui16 y, ui8 color);
void DisplayLine(int x0, int y0, int x1, int y1, ui8 color);
void DisplayRect(int x0, int y0, int width, int height, ui8 color);
void DisplayFillRect(ui16 left, ui16 top, ui16 width, ui16 height, ui8 color);
void DisplayRomb(ui16 centerX, ui16 centerY, ui16 widthHeight, ui8 color);
void DisplayFillRomb(ui16 centerX, ui16 centerY, ui16 widthHeight, ui8 color);
void DisplayCircle(int xm, int ym, int r, ui8 color);
void DisplayFillCircle(int x0, int y0, int r, ui8 color);
void DisplayTrian(int x0, int y0, int x1, int y1, int x2, int y2, ui8 color);

void CopyTileToScreen(const void *tile, int x, int y);

void DisplayText(const char* s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color);
void DisplayTextW(const WIDE_CHAR* s, unsigned int x, unsigned int y, unsigned int fontType, ui8 color);

void DisplayRainbow(void);
void DisplayTest(void);

#endif //__GRAPH_H__
