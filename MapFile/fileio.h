#ifndef __SD_H__
#define __SD_H__
#include "types.h"
#include "sizes.h"

#ifdef __cplusplus
extern "C"
{
#endif
 bool file_create(void);
 bool file_read(FileAddr addr, void *dst, ui32 sz);
 bool file_write(FileAddr addr, const void *src, ui32 sz);
#ifdef __cplusplus
}
#endif

#endif //!__SD_H__
