#define _CRT_SECURE_NO_WARNINGS
#include "device.h"
#include "fs.h"
#include "coord.h"
#include "convert.h"
#include "lodepng.h"

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <assert.h>

Device::Device(int id_) : id(id_)
{
	for (int i = 0; i < sizeof(map) / sizeof(*map); ++i)
		map[i] = (ui8*)malloc(TILE_CX * TILE_CY / 2);
}

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
	ui8* p = (ui8*)malloc(TILE_CX * TILE_CY + BLOCK_SIZE); // round up to BLOCK_SIZE
	ui32 sz = Compress4BitBuffer(p4, p + sizeof(ui32));
	*(ui32*)p = sz + sizeof(sz);
	return (NewTile*)p;
}

static void forgetTile(void* p)
{
	free(p);
}

void Device::init()
{
	fsFormat(id);

	IMS ims;
	BlockAddr addr;
	RectFloat r;
	r.left = 38.0f;
	r.right = 38.4f;
	r.bottom = 55.95f;
	r.top = 56.2f;
	fsAddIMS(&ims, &addr, &r, id);

	for (ui8 z = 12; z < 13; ++z)
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

void Device::processGps(PointFloat point)
{
	static IMS ims = { 0 };
	const ui8 zoom = 12;
	if (!PointInRect(&ims.coord, point.x, point.y))
		fsFindIMS(point.x, point.y, &ims, id);
	currentTilePos.x = lon2tilex(point.x, zoom);
	currentTilePos.y = lat2tiley(point.y, zoom);
	auto addr = fsFindTile(&ims, zoom, (ui32)currentTilePos.x, (ui32)currentTilePos.y, id);
	if (addr)
	{
		ui8* tile = (ui8*)malloc(TILE_CX * TILE_CY / 2 + BLOCK_SIZE + sizeof(ui32));
		fsReadTile(addr, tile, id); // TODO: implement decompress on the fly
		ui32 sz = *(ui32*)tile;
		DeCompress(tile + sizeof(ui32), map[0]);
		free(tile);
	}
	else
	{
		memset(map[0], 0, TILE_CX * TILE_CY / 2);
	}
	int xx = (currentTilePos.x - (ui32)currentTilePos.x) * TILE_CX;
	xx -= SCREEN_CX / 2;
	int yy = (currentTilePos.y - (ui32)currentTilePos.y) * TILE_CY;
	yy -= SCREEN_CY / 2;
	yy /= 2;
	memset(screen, 0, SCREEN_CX * SCREEN_CY / 2);
	for (int i = 0; i < TILE_CX; ++i)
		for (int j = 0; j < TILE_CY / 2; ++j)
		{
			int ii = i - xx;
			int jj = j - yy;
			if ((ii >= 0) && (ii < SCREEN_CX))
				if ((jj >= 0) && (jj < SCREEN_CY / 2))
					screen[ii][jj] = *(map[0] + i * TILE_CY / 2 + j);
		}
//	memcpy(screen, map[0], TILE_CX * c / 2);
	redrawScreen = true;
}

void Device::run()
{
	while (gps.size())
	{
		processGps(gps.front());
		gps.pop_front();
	}
}
