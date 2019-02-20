#ifndef __SD_H__
#define __SD_H__
#include "types.h"
#include "sizes.h"

static inline BlockAddr sdCardSize(void)
{
	return 1024 * 16;
}

void sdCardRead(BlockAddr addr, void *dst, int id);
void sdCardWrite(BlockAddr addr, const void *src, int id);

#define CREATE_NEW_SD 0

#endif //!__SD_H__