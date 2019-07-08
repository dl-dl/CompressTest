#ifndef __SD_H__
#define __SD_H__
#include "types.h"
#include "sizes.h"

#ifdef __cplusplus
extern "C"
{
#endif
 bool file_open(const char* name, bool write);
 bool file_write(FileAddr addr, const void *src, ui32 sz);
 void file_close(void);
#ifdef __cplusplus
}
#endif

#endif //!__SD_H__
