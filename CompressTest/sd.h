#ifndef __SD_H__
#define __SD_H__
#include "types.h"

typedef ui32 BlockAddr;
static const ui32 BLOCK_SIZE = 512;

static inline BlockAddr sdCardSize(void)
{
	return 1024 * 10;
}

void sdCardRead(BlockAddr addr, void *dst, int id);
void sdCardWrite(BlockAddr addr, const void *src, int id);

#endif //!__SD_H__