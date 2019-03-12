#ifndef __SD_H__
#define __SD_H__
#include "types.h"
#include "sizes.h"

static inline BlockAddr SDCardSize(void)
{
 return 1024 * 16; // Number of blocks.
}

#ifdef __cplusplus
extern "C"
{
#endif
 bool SDCardRead(BlockAddr addr, void *dst, ui32 numBlocks, int id);
 bool SDCardWrite(BlockAddr addr, const void *src, ui32 numBlocks, int id);
#ifdef __cplusplus
}
#endif

#define CREATE_NEW_SD 0

#endif //!__SD_H__