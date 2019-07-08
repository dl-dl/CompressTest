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
		return RES_OK;
	const char fname[] = "FAT32.BIN";
	file_handle = CreateFileA(fname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file_handle != INVALID_HANDLE_VALUE)
		return RES_OK;
	return RES_ERROR;
}

DSTATUS disk_status(BYTE pdrv)
{
	return RES_OK;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
	DWORD fp = SetFilePointer(file_handle, sector * BLOCK_SIZE, NULL, FILE_BEGIN);
	DWORD n;
	BOOL res = ReadFile(file_handle, buff, BLOCK_SIZE * count, &n, NULL);
	if(res && (n == BLOCK_SIZE * count))
		return RES_OK;
	return RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
	DWORD fp = SetFilePointer(file_handle, sector * BLOCK_SIZE, NULL, FILE_BEGIN);
	DWORD n;
	BOOL res = WriteFile(file_handle, buff, BLOCK_SIZE * count, &n, NULL);
	return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
	if (cmd == GET_SECTOR_COUNT)
		* (DWORD*)buff = 1024 * 1024 * 2;
	else if (cmd == GET_BLOCK_SIZE)
		* (DWORD*)buff = 1;
	return RES_OK;
}
