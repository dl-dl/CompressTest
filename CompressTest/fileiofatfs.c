#define _CRT_SECURE_NO_WARNINGS
#include "fileio.h"
#include "ff.h"
//#define FORAMT_FAT32
#ifdef FORAMT_FAT32
#include <stdio.h>
#endif

static FIL fil;
static FATFS fs;

void MapInitFS()
{
 const TCHAR path[] = "0:/";
#ifdef FORAMT_FAT32
 static BYTE buff[512 * 32];
 f_mkfs(path, FM_FAT32, 512, &buff, sizeof(buff));
 f_mount(&fs, path, 1);

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
   fclose(wf);
  }
#else
 f_mount(&fs, path, 1);
#endif
}

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
 UINT n = 0;
 f_lseek(&fil, addr);
 f_write(&fil, src, sz, &n);
 return n == sz;
}

static FATFS_DIR dj;
static FILINFO fno;

bool file_open_dir()
{
 return (FR_OK == f_opendir(&dj, ""));
}

const char *file_read_dir()
{
 if (FR_OK != f_readdir(&dj, &fno))
  return 0;
 if (0 == *fno.fname)
  return 0;
 if (fno.fattrib & AM_DIR)
  return 0;
 return fno.fname;
}

void file_close_dir()
{
 f_closedir(&dj);
}
