#ifndef __SIZES_H_
#define __SIZES_H_

#include "types.h"

#define SCREEN_CX 240
#define SCREEN_CY 400

#define TILE_CX 256
#define TILE_CY 256

#define DEV_RED 0x08
#define DEV_GREEN 0x04
#define DEV_BLUE 0x02
#define DEV_WHITE (DEV_RED|DEV_GREEN|DEV_BLUE)

typedef ui32 BlockAddr;
#define BLOCK_SIZE 512

#ifdef _WINDOWS
#define NUM_DEV 3
#else
#define NUM_DEV 1
#endif

#endif // __SIZES_H_