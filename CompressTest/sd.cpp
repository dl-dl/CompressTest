#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#ifdef _WINDOWS
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
#include "sd.h"
#include "sizes.h"

static HANDLE sdopen(int id)
{
	static HANDLE file_handles[8] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
		INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
	assert(id < sizeof(file_handles) / sizeof(*file_handles));
	if (file_handles[id] == INVALID_HANDLE_VALUE)
	{
		char fname[] = "SD .BIN";
		fname[2] = id + '0';
#ifdef CREATE_NEW_SD
		file_handles[id] = CreateFileA(fname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
		file_handles[id] = CreateFileA(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
		assert(file_handles[id] != INVALID_HANDLE_VALUE);
	}
	return file_handles[id];
}

static void sdclose(HANDLE file)
{
//	CloseHandle(file);
}

void sdCardRead(BlockAddr addr, void *dst, int id)
{
	assert(addr < sdCardSize());
	HANDLE f = sdopen(id);
	DWORD fp = SetFilePointer(f, addr*BLOCK_SIZE, NULL, FILE_BEGIN);
	assert(fp == addr * BLOCK_SIZE);
	DWORD n = 0;
	ReadFile(f, dst, BLOCK_SIZE, &n, NULL);
	assert(n == BLOCK_SIZE);
	sdclose(f);
}

void sdCardWrite(BlockAddr addr, const void *src, int id)
{
	assert(addr < sdCardSize());
	HANDLE f = sdopen(id);
	DWORD fp = SetFilePointer(f, addr*BLOCK_SIZE, NULL, FILE_BEGIN);
	assert(fp == addr * BLOCK_SIZE);
	DWORD n = 0;
	WriteFile(f, src, BLOCK_SIZE, &n, NULL);
	assert(n == BLOCK_SIZE);
	sdclose(f);
}

#endif
