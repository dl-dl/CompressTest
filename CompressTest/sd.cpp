#include <assert.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <Windows.h>
#include "sd.h"

static HANDLE sdopen(int id)
{
	static HANDLE file_handles[16];
	assert(id < sizeof(file_handles) / sizeof(*file_handles));
	if (!file_handles[id])
	{
		char fname[] = "SD .BIN";
		fname[2] = id + '0';
		file_handles[id] = CreateFileA(fname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
	SetFilePointer(f, addr*BLOCK_SIZE, NULL, FILE_BEGIN);
	if (!ReadFile(f, dst, BLOCK_SIZE, NULL, NULL))
		memset(dst, 0xDF, BLOCK_SIZE);
	sdclose(f);
}

void sdCardWrite(BlockAddr addr, const void *src, int id)
{
	assert(addr < sdCardSize());
	HANDLE f = sdopen(id);
	SetFilePointer(f, addr*BLOCK_SIZE, NULL, FILE_BEGIN);
	WriteFile(f, src, BLOCK_SIZE, NULL, NULL);
	sdclose(f);
}

#endif
