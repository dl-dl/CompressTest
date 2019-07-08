#include "fileio.h"
#include "ff.h"

#ifdef TRACE_READ
#include <stdio.h>
#endif

static FIL fil;

bool file_create(const char *name)
{
 FIL newfile;
 if (FR_OK != f_open(&newfile, name, FA_WRITE | FA_CREATE_ALWAYS))
  return false;
 f_close(&newfile);
 return true;
}

bool file_open(const char *name, bool write)
{
 if (FR_OK != f_open(&fil, name, write ? FA_WRITE : FA_READ))
  return false;
 return true;
}

void file_close()
{
 f_close(&fil);
}

bool file_read(FileAddr addr, void *dst, ui32 sz)
{
 UINT n = 0;
 f_lseek(&fil, addr);
 f_read(&fil, dst, sz, &n);
 return n == sz;
}

bool file_write(FileAddr addr, const void *src, ui32 sz)
{
 FIL fil;
 UINT n = 0;
 f_lseek(&fil, addr);
 f_write(&fil, src, sz, &n);
 return n == sz;
}
