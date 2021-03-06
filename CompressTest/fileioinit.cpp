#include "fileio.h"
#include "ff.h"

#if FF_FS_READONLY == 0

#include <stdio.h>

extern "C" FATFS fs;

bool CreateFileSys()
{
 static BYTE buff[512 * 32];
 if (FR_OK != f_mkfs("", FM_FAT32, 512*16, &buff, sizeof(buff)))
  return false;
 if (FR_OK != f_mount(&fs, "", 1))
  return false;

 file_create("f1.map");
 FILE *wf = fopen("f1.bin", "rb");
 if (file_open("f1.map", true))
  {
   ui32 n;
   ui32 addr = 0;
   do
    {
     n = fread(buff, 1, 512, wf);
     file_write(addr, buff, n);
     addr += n;
    }
   while (n == 512);

   file_close();
  }
 fclose(wf);
 return true;
}

#endif
