#include "ff.h"
#include "diskio.h"

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>

static HANDLE file_handle = INVALID_HANDLE_VALUE;
static const unsigned int BLOCK_SIZE = 512;

DSTATUS disk_initialize(BYTE pdrv)
{
 if (file_handle != INVALID_HANDLE_VALUE)
  return 0;
 file_handle = CreateFileA("FAT32.BIN", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 if (file_handle == INVALID_HANDLE_VALUE)
  return STA_NODISK;
 return 0;
}

DSTATUS disk_status(BYTE pdrv)
{
 if (file_handle == INVALID_HANDLE_VALUE)
  return STA_NODISK;
 return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
 if (INVALID_SET_FILE_POINTER == SetFilePointer(file_handle, sector * BLOCK_SIZE, NULL, FILE_BEGIN))
  return RES_ERROR;
 DWORD n;
 BOOL res = ReadFile(file_handle, buff, count * BLOCK_SIZE, &n, NULL);
 if (res && (n == BLOCK_SIZE * count))
  return RES_OK;
 return RES_ERROR;
}

#if !FF_FS_READONLY
DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
 if (INVALID_SET_FILE_POINTER == SetFilePointer(file_handle, sector * BLOCK_SIZE, NULL, FILE_BEGIN))
  return RES_ERROR;
 DWORD n;
 BOOL res = WriteFile(file_handle, buff, count * BLOCK_SIZE, &n, NULL);
 return RES_OK;
}
#endif

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
 if (cmd == GET_SECTOR_COUNT)
  *(DWORD *)buff = 16 * 2 * 1024 * 1024;
 else if (cmd == GET_BLOCK_SIZE)
  *(DWORD *)buff = 1;
 else if (buff)
  return RES_ERROR;
 return RES_OK;
}
