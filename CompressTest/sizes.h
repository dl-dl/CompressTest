#ifndef __SIZES_H_
#define __SIZES_H_

#define SCREEN_CX 240
#define SCREEN_CY 400

#define TILE_CX 256
#define TILE_CY 256

#define DEV_RED 0x02
#define DEV_GREEN 0x04
#define DEV_BLUE 0x08

typedef ui32 BlockAddr;
static const ui32 BLOCK_SIZE = 512;

#endif // __SIZES_H_