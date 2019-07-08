#include "fileio.h"

#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOSERVICE
#include <windows.h>

static HANDLE fil = INVALID_HANDLE_VALUE;

bool file_create(const char *name)
{
 HANDLE f = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 if (f == INVALID_HANDLE_VALUE)
  return false;
 CloseHandle(f);
 return true;
}

bool file_open(const char *name, bool write)
{
 assert(fil == INVALID_HANDLE_VALUE);
 fil = CreateFileA(name, write ? GENERIC_WRITE : GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 if (fil == INVALID_HANDLE_VALUE)
  return false;
 return true;
}

void file_close()
{
 assert(fil != INVALID_HANDLE_VALUE);
 CloseHandle(fil);
 fil = INVALID_HANDLE_VALUE;
}

bool file_read(FileAddr addr, void *dst, ui32 sz)
{
 SetFilePointer(fil, addr, NULL, FILE_BEGIN);
 DWORD n;
 BOOL res = ReadFile(fil, dst, sz, &n, NULL);
 assert(res);
 assert(n == sz);
 return res && (n == sz);
}

bool file_write(FileAddr addr, const void *src, ui32 sz)
{
 SetFilePointer(fil, addr, NULL, FILE_BEGIN);
 DWORD n;
 BOOL res = WriteFile(fil, src, sz, &n, NULL);
 assert(res);
 assert(n == sz);
 return res && (n == sz);
}
