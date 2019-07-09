#define _CRT_SECURE_NO_WARNINGS
#include "fileiostd.h"
#include <assert.h>
#include <stdio.h>

static FILE *fil = NULL;

bool map_file_open(const char *name, bool write)
{
 assert(fil == NULL);
 fil = fopen(name, "wb");
 return fil != NULL;
}

void map_file_close()
{
 assert(fil);
 fclose(fil);
 fil = NULL;
}

bool map_file_write(FileAddr addr, const void *src, ui32 sz)
{
 fseek(fil, addr, SEEK_SET);
 size_t n = fwrite(src, 1, sz, fil);
 assert(n == sz);
 return n == sz;
}
