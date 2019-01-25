#ifndef __GRAPH_H__
#define __GRAPH_H__
#include "types.h"

#define DEV_RED 0x01
#define DEV_GREEN 0x02
#define DEV_BLUE 0x04

struct PaintContext;

void gPixel(const PaintContext* ctx, int x, int y, ui8 color);
void gLine(const PaintContext* ctx, int x0, int y0, int x1, int y1, int color);
void gCircle(const PaintContext* ctx, int x, int y, int r, ui8 color);
void gText(const PaintContext* ctx, int x, int y, ui8 color);

#endif