#ifndef __PAINT_H
#define __PAINT_H

#include "types.h"
#include "screen.h"

struct PaintContext;

void PaintScreen(const PaintContext* ctx, const Screen* screen);

#endif
