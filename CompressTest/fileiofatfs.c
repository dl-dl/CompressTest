#include "fileio.h"
#include "ff.h"

static FIL fil;
FATFS fs;
static DIR dj;
static FILINFO fno;

static bool fsMounted;

bool InitFileSys()
{
 if (!fsMounted)
  fsMounted = (FR_OK == f_mount(&fs, "", 1));
 return fsMounted;
}

void DeinitFileSys()
{
 if (fsMounted)
  {
   file_close();
   f_mount(0, "", 1);
   fsMounted = false;
  }
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
 if (FR_OK != f_lseek(&fil, addr))
  return false;
 UINT n;
 if (FR_OK != f_read(&fil, dst, sz, &n))
  return false;
 return n == sz;
}

bool file_optimize_read()
{
#if FF_USE_FASTSEEK && FF_FS_READONLY
 static DWORD clmt[64];
 clmt[0] = sizeof(clmt) / sizeof(clmt[0]);
 fil.cltbl = clmt;
 if (FR_OK != f_lseek(&fil, CREATE_LINKMAP))
  {
   fil.cltbl = 0;
   return false;
  }
#endif
 return true;
}

#if !FF_FS_READONLY
bool file_create(const char *name)
{
 FIL newfile;
 if (FR_OK != f_open(&newfile, name, FA_WRITE | FA_CREATE_ALWAYS))
  return false;
 f_close(&newfile);
 return true;
}

bool file_write(FileAddr addr, const void *src, ui32 sz)
{
 if (FR_OK != f_lseek(&fil, addr))
  return false;
 UINT n;
 if (FR_OK != f_write(&fil, src, sz, &n))
  return false;
 return n == sz;
}
#endif

bool file_open_dir()
{
 return (FR_OK == f_opendir(&dj, ""));
}

const char *file_read_dir() // returns pointer to static memory
{
 if (FR_OK != f_readdir(&dj, &fno))
  return 0;
 if (0 == *fno.fname)
  return 0;
 if (fno.fattrib & AM_DIR)
  return "";
 return fno.fname;
}

void file_close_dir()
{
 f_closedir(&dj);
}
