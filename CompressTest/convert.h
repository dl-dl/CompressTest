#ifndef __CONVERT_H
#define __CONVERT_H

#include "types.h"

#define TILE_CX 256
#define TILE_CY 256

// PNG layout to normal
void Invert24(const void* src, void* dst);
void Invert8(const void* src, void* dst);
// src in PNG layout (x <-> y)
void ConvertInv24To8Approx(const void* src, void* dst);
// src in normal layout
void Convert24To8Approx(const void* src, void* dst);
void Convert24To8(const void* src, void* dst);
void Convert8To4(const void* src, void* dst);
void Convert4To24(const void* src, void *dst);
void Convert8To24(const void* src, void *dst);

unsigned int Compress4BitBuffer(const void* src, void* dst);
void DeCompress(const void* src, void* dst);

void RGB2XYZ(ui8 &bCol, ui8 &gCol, ui8 &rCol);

#endif
