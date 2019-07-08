#include "fileio.h"

#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOSERVICE
#include <windows.h>

bool file_create()
{
 HANDLE f = CreateFileA("f1.bin", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 if (f == INVALID_HANDLE_VALUE)
  return false;
 CloseHandle(f);
 return true;
}

bool file_read(FileAddr addr, void *dst, ui32 sz)
{
 HANDLE f = CreateFileA("f1.bin", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 if (f == INVALID_HANDLE_VALUE)
  return false;
 SetFilePointer(f, addr, NULL, FILE_BEGIN);
 DWORD n;
 BOOL res = ReadFile(f, dst, sz, &n, NULL);
 CloseHandle(f);
 assert(res);
 assert(n == sz);
 return res && (n == sz);
}

bool file_write(FileAddr addr, const void *src, ui32 sz)
{
 HANDLE f = CreateFileA("f1.bin", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 if (f == INVALID_HANDLE_VALUE)
  return false;
 SetFilePointer(f, addr, NULL, FILE_BEGIN);
 DWORD n;
 BOOL res = WriteFile(f, src, sz, &n, NULL);
 CloseHandle(f);
 assert(res);
 assert(n == sz);
 return res && (n == sz);
}
