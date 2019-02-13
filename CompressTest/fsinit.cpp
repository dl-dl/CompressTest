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

struct NewTile
{
	ui32 size;
	ui8* data;
};

static NewTile getTile(int x, int y, ui8 z, const char* region)
{
	char path[1024];
	sprintf(path, "C:\\tmp\\%s\\%u\\%u\\%u.png", region, z, x, y);
	NewTile t = { 0, 0 };

	FILE* f = fopen(path, "rb");
	if (!f)
		return t;
	fseek(f, 0, SEEK_END);
	size_t pngsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	ui8* png = (unsigned char*)malloc(pngsize);
	if (NULL == png)
		return t;
	fread(png, 1, pngsize, f);
	fclose(f);

	unsigned width, height;
	ui8* b;
	unsigned error = lodepng_decode_memory(&b, &width, &height, png, pngsize, LCT_RGB, 8);
	if (error)
		return t;
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
	t.data = (ui8*)malloc(TILE_CX * TILE_CY + BLOCK_SIZE); // round up to BLOCK_SIZE
	t.size = Compress4BitBuffer(p4, t.data);
	return t;
}

static void forgetTile(NewTile t)
{
	free(t.data);
}

void FsInit(int id)
{
	IMS ims;
	BlockAddr addr;
	RectFloat r[1];
	r[0].left = 38.1f;
	r[0].right = 38.6f;
	r[0].top = 56.1f;
	r[0].bottom = 55.95f;
/*	r[1].left = -71.65f;
	r[1].right = -71.4f;
	r[1].top = -33.0f;
	r[1].bottom = -33.1f;
*/	const char* region[2] = { "cher", "valparaiso" };

	for (int i = 0; i < sizeof(r)/sizeof(*r); ++i)
	{
		FsNewIMS(&ims, &addr, r + i, id);

		for (ui8 z = 12; z <= 14; ++z)
		{
			NewMapStatus status;
			ImsNextZoom(&ims, &status, z);
			int startX = (int)lon2tilex(r[i].left, z);
			int startY = (int)lat2tiley(r[i].top, z);
			int stopX = (int)lon2tilex(r[i].right, z);
			int stopY = (int)lat2tiley(r[i].bottom, z);
			for (int x = startX; x <= stopX; ++x)
				for (int y = startY; y <= stopY; ++y)
				{
					NewTile tile = getTile(x, y, z, region[i]);
					assert(tile.data);
					bool res = ImsAddTile(&ims, &status, tile.data, tile.size, id);
					assert(res);
					forgetTile(tile);
				}
			FsCommitIMS(&ims, addr, id);
		}
	}
}
