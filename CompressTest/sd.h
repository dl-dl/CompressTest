#ifndef __SD_H__
#define __SD_H__
#include "types.h"
#include "sizes.h"

#ifdef __cplusplus
extern "C"
{
#endif
 bool SDCardMapRead(BlockAddr addr, void *dst, ui32 numBlocks);
 bool SDCardMapWrite(BlockAddr addr, const void *src, ui32 numBlocks);
#ifdef __cplusplus
}
#endif

#define CREATE_NEW_SD 0

#endif //!__SD_H__
