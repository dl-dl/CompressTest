#include "fileio.h"
#include "ff.h"

#ifdef TRACE_READ
#include <stdio.h>
#endif
/*
	FIL fil;
	const TCHAR fname[] = "f1.txt";
	UINT n = 0;
	f_open(&fil, fname, FA_WRITE | FA_CREATE_ALWAYS);
	const char xxx[12] = "Hello FIL";
	f_write(&fil, xxx, 12, &n);
	f_close(&fil);

	f_open(&fil, fname, FA_READ);
	f_lseek(&fil, 0);
	char yyy[10];
	f_read(&fil, yyy, 10, &n);
	f_close(&fil);
	*/
static const TCHAR fname[] = "f1.txt";

bool file_create()
{
 FIL fil;
 if (FR_OK != f_open(&fil, fname, FA_WRITE | FA_CREATE_ALWAYS))
  return false;
 f_close(&fil);
 return true;
}

bool file_read(FileAddr addr, void *dst, ui32 sz)
{
 FIL fil;
 UINT n = 0;
 if (FR_OK != f_open(&fil, fname, FA_READ))
  return false;
 f_lseek(&fil, addr);
 f_read(&fil, dst, sz, &n);
 f_close(&fil);
 return n == sz;
}

bool file_write(FileAddr addr, const void *src, ui32 sz)
{
 FIL fil;
 UINT n = 0;
 if (FR_OK != f_open(&fil, fname, FA_READ))
  return false;
 f_lseek(&fil, addr);
 f_write(&fil, src, sz, &n);
 f_close(&fil);
 return n == sz;
}
