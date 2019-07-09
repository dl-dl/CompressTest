#ifndef __FILEIOSTD_H__
#define __FILEIOSTD_H__
#include "types.h"
#include "sizes.h"

bool map_file_open(const char *name, bool write);
bool map_file_write(FileAddr addr, const void *src, ui32 sz);
void map_file_close(void);

#endif //!__FILEIOSTD_H__
