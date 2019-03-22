#ifndef __SIZES_H_
#define __SIZES_H_

#include "types.h"

#define SCREEN_DX 240
#define SCREEN_DY 400

#define TILE_DX 256
#define TILE_DY 256

#define DEV_RED 0x08
#define DEV_GREEN 0x04
#define DEV_BLUE 0x02
#define DEV_WHITE (DEV_RED|DEV_GREEN|DEV_BLUE)

typedef ui32 BlockAddr;
#define BLOCK_SIZE 512

#endif // __SIZES_H_