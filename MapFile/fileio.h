#ifndef __SD_H__
#define __SD_H__
#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif
 bool MapInitFS(void);
 bool MapCreateFS(void);

 bool file_create(const char* name);
 bool file_open(const char* name, bool write);
 bool file_read(FileAddr addr, void *dst, ui32 sz);
 bool file_write(FileAddr addr, const void *src, ui32 sz);
 void file_close(void);
 bool file_optimize_read(void);

 bool file_open_dir(void);
 const char* file_read_dir(void);
 void file_close_dir(void);
#ifdef __cplusplus
}
#endif

#endif //!__SD_H__
