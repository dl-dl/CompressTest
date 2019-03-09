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
 void SDCardRead(BlockAddr addr, void *dst, int id);
 void SDCardWrite(BlockAddr addr, const void *src, int id);
#ifdef __cplusplus
}
#endif

#define CREATE_NEW_SD 0

#endif //!__SD_H__