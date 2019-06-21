#include "sd.h"

#ifdef _WINDOWS
#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>

static HANDLE sdopen()
{
 static HANDLE file_handle = INVALID_HANDLE_VALUE;
 if (file_handle == INVALID_HANDLE_VALUE)
  {
   const char fname[] = "SD0.BIN";
#if CREATE_NEW_SD
   file_handle = CreateFileA(fname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   ui8 b[BLOCK_SIZE];
   memset(b, 0, BLOCK_SIZE);
   for (BlockAddr i = 0; i < MAP_SIZE; ++i)
    {
     DWORD n;
     BOOL res = WriteFile(file_handle, b, BLOCK_SIZE, &n, NULL);
     assert(res);
     assert(n == BLOCK_SIZE);
    }
#else
   file_handle = CreateFileA(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
   assert(file_handle != INVALID_HANDLE_VALUE);
  }
 return file_handle;
}

static void sdclose(HANDLE file)
{
 //	CloseHandle(file);
}

#ifdef TRACE_SD
#include <stdio.h>
#endif

bool SDCardMapRead(BlockAddr addr, void *dst, ui32 numBlocks)
{
#ifdef TRACE_SD
 static FILE *log = fopen("SDLOG.txt", "wt");
 fprintf(log, "%08X\n", addr);
#endif
 HANDLE f = sdopen();
 DWORD fp = SetFilePointer(f, addr * BLOCK_SIZE, NULL, FILE_BEGIN);
 assert(fp == addr * BLOCK_SIZE);
 DWORD n;
 BOOL res = ReadFile(f, dst, BLOCK_SIZE * numBlocks, &n, NULL);
 sdclose(f);
 assert(res);
 assert(n == BLOCK_SIZE);
 return n == BLOCK_SIZE;
}

bool SDCardMapWrite(BlockAddr addr, const void *src, ui32 numBlocks)
{
 HANDLE f = sdopen();
 DWORD fp = SetFilePointer(f, addr * BLOCK_SIZE, NULL, FILE_BEGIN);
 assert(fp == addr * BLOCK_SIZE);
 DWORD n;
 BOOL res = WriteFile(f, src, BLOCK_SIZE * numBlocks, &n, NULL);
 sdclose(f);
 assert(res);
 assert(n == BLOCK_SIZE);
 return n == BLOCK_SIZE;
}

#endif
