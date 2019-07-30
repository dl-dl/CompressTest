#ifndef __MAP_H_
#define __MAP_H_

#include "coord.h"

void MapInit(void);
void MapDeinit(void);

void DrawMap(void);
void DrawGroup(void);

void AdjustScreenPos(ui32 x, ui32 y);
PointInt GetScreenCenter(void);

#endif
