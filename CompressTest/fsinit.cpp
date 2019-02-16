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
	RectInt r[2];
	r[0].left = lon2tilex(38.1f, MAX_ZOOM_LEVEL) / TILE_CX;
	r[0].right = lon2tilex(38.6f, MAX_ZOOM_LEVEL) / TILE_CX;
	r[0].top = lat2tiley(56.1f, MAX_ZOOM_LEVEL) / TILE_CY;
	r[0].bottom = lat2tiley(55.95f, MAX_ZOOM_LEVEL) / TILE_CY;

	r[1].left = lon2tilex(-71.65f, MAX_ZOOM_LEVEL) / TILE_CX;
	r[1].right = lon2tilex(-71.4f, MAX_ZOOM_LEVEL) / TILE_CX;
	r[1].top = lat2tiley(-33.0f, MAX_ZOOM_LEVEL) / TILE_CY;
	r[1].bottom = lat2tiley(-33.1f, MAX_ZOOM_LEVEL) / TILE_CY;
	const char* region[2] = { "cher", "valparaiso" };

	for (int i = 0; i < sizeof(r)/sizeof(*r); ++i)
	{
		FsNewIMS(&ims, &addr, r + i, id);

		for (ui8 z = 12; z <= 14 - i; ++z)
		{
			NewMapStatus status;
			ImsNextZoom(&ims, &status, z);
			const ImsIndexDescr* idx = &ims.index[z - MIN_ZOOM_LEVEL];
			for (ui32 x = 0; x < idx->nx; ++x)
				for (ui32 y = 0; y < idx->ny; ++y)
				{
					NewTile tile = getTile(idx->left + x, idx->top + y, z, region[i]);
					assert(tile.data);
					bool res = ImsAddTile(&ims, &status, tile.data, tile.size, id);
					assert(res);
					forgetTile(tile);
				}
		}
		FsCommitIMS(&ims, addr, id);
	}
}
