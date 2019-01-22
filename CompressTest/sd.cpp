#include "sd.h"
#include <assert.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static HANDLE file = INVALID_HANDLE_VALUE;

static void init(char id)
{
	char fname[] = "SD .BIN";
	fname[2] = id;
	if (file == INVALID_HANDLE_VALUE)
		file = CreateFileA(fname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

void sdCardRead(BlockAddr addr, void *dst, char id)
{
	assert(addr < sdCardSize());
	init(id);
	SetFilePointer(file, addr*BLOCK_SIZE, NULL, FILE_BEGIN);
	if (!ReadFile(file, dst, BLOCK_SIZE, NULL, NULL))
		memset(dst, 0xDF, BLOCK_SIZE);
}

void sdCardWrite(BlockAddr addr, const void *src, char id)
{
	assert(addr < sdCardSize());
	init(id);
	SetFilePointer(file, addr*BLOCK_SIZE, NULL, FILE_BEGIN);
	WriteFile(file, src, BLOCK_SIZE, NULL, NULL);
}
