#define _CRT_SECURE_NO_WARNINGS
#include "types.h"
#include "fsinit.h"
#include "fs.h"
#include "sizes.h"
#include "convert.h"

#include "lodepng.h"

#include <malloc.h>
#include <assert.h>
#include <stdio.h>

static NewTile* getTile(int x, int y, ui8 z)
{
	char path[1024];
	sprintf(path, "C:\\tmp\\%u\\%u\\%u.png", z, x, y);

	FILE* f = fopen(path, "rb");
	if (!f)
		return NULL;
	fseek(f, 0, SEEK_END);
	size_t pngsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	ui8* png = (unsigned char*)malloc(pngsize);
	if (NULL == png)
		return NULL;
	fread(png, 1, pngsize, f);
	fclose(f);

	unsigned width, height;
	ui8* b;
	unsigned error = lodepng_decode_memory(&b, &width, &height, png, pngsize, LCT_RGB, 8);
	if (error)
		return NULL;
	free(png);

	ui8* p24 = (ui8*)malloc(TILE_CX * TILE_CY * 3);
	Invert24(b, p24);
	free(b);

	ui8* p8 = (ui8*)malloc(TILE_CX * TILE_CY);
	Convert24To8(p24, p8);
	free(p24);
	ui8* p4 = (ui8*)malloc(TILE_CX * TILE_CY / 2);
	Convert8To4(p8, p4);
	free(p8);
	NewTile* p = (NewTile*)malloc(TILE_CX * TILE_CY + BLOCK_SIZE); // round up to BLOCK_SIZE
	ui32 sz = Compress4BitBuffer(p4, p->data);
	p->size = sz + sizeof(p->size);
	return (NewTile*)p;
}

static void forgetTile(void* p)
{
	free(p);
}

void fsInit(int id)
{
	IMS ims;

	BlockAddr addr;
	RectFloat r;
	r.left = 38.0f;
	r.right = 38.4f;
	r.bottom = 55.95f;
	r.top = 56.2f;
	fsAddIMS(&ims, &addr, &r, id);

	for (ui8 z = 12; z <= 13; ++z)
	{
		NewMapStatus status;
		imsNextZoom(&ims, &status, z);
		for (float x = lon2tilex(r.left, z); x <= lon2tilex(r.right, z); ++x)
			for (float y = lat2tiley(r.top, z); y <= lat2tiley(r.bottom, z); ++y)
			{
				NewTile* tile = getTile((int)x, (int)y, z);
				bool res = imsAddTile(&ims, &status, tile, id);
				assert(res);
				forgetTile(tile);
			}
		fsCommitIMS(&ims, addr, id);
	}
}
