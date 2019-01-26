#define _CRT_SECURE_NO_WARNINGS
#include "device.h"
#include "fs.h"
#include "coord.h"
#include "convert.h"
#include "graph.h"
#include "paint.h"
#include "lodepng.h"

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <assert.h>

Device::Device()
{
	for (int i = 0; i < sizeof(mapCache) / sizeof(*mapCache); ++i)
	{
		mapCache[i].zoom = 0; // empty
	}
	memset(&ims, 0, sizeof(ims));
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

void Device::init(int id_)
{
	id = id_;
	fsFormat(id);

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

static inline ui32 cacheIndex(ui32 x, ui32 y)
{
	return (x % 2) + (y % 3) * 2;
}

static inline bool cacheEq(const MapCacheItem* p, ui32 x, ui32 y, ui32 z)
{
	return (p->tileX == x) && (p->tileY == y) && (p->zoom == z);
}

ui32 Device::cacheRead(const IMS* ims, ui32 tileX, ui32 tileY, ui32 zoom)
{
	ui32 index = cacheIndex(tileX, tileY);
	if (!cacheEq(&mapCache[index], tileX, tileY, zoom))
	{
		auto addr = fsFindTile(ims, zoom, tileX, tileY, id);
		if (addr)
		{
			ui8* tile = (ui8*)malloc(TILE_CX * TILE_CY / 2 + BLOCK_SIZE + sizeof(ui32));
			fsReadTile(addr, tile, id); // TODO: implement decompress on the fly
			ui32 sz = ((NewTile*)tile)->size;
			DeCompress(((NewTile*)tile)->data, mapCache[index].data);
			free(tile);
		}
		else
		{
			memset(mapCache[index].data, 0, TILE_CX * TILE_CY / 2);
		}
		mapCache[index].zoom = zoom;
		mapCache[index].tileX = tileX;
		mapCache[index].tileY = tileY;
	}
	return index;
}

void Device::processGps(PointFloat point)
{
	const ui8 zoom = 12;
	currentTile.x = lon2tilex(point.x, zoom);
	currentTile.y = lat2tiley(point.y, zoom);
	ui32 tileX = (ui32)currentTile.x;
	ui32 tileY = (ui32)currentTile.y;

	int xx = (currentTile.x - tileX) * TILE_CX; // screen offset x
	xx = SCREEN_CX / 2 - xx;
	int yy = (currentTile.y - tileY) * TILE_CY; // screen offset y
	yy = SCREEN_CY / 2 - yy;
	yy /= 2;
	memset(&screen, 0, sizeof(screen));

	if (!PointInRect(&ims.coord, point.x, point.y))
		fsFindIMS(point.x, point.y, &ims, id);

	int dxt = 0;
	if (xx > 0)
		dxt = -1;
	else if (xx + TILE_CX < SCREEN_CX)
		dxt = 1;
	for (int i = 0; i <= abs(dxt); ++i)
	{
		ui32 index = cacheRead(&ims, tileX + i * dxt, tileY, zoom);
		CopyTileToScreen(mapCache[index].data, xx + i * dxt * TILE_CX, yy, &screen);
		if (yy > 0)
		{
			index = cacheRead(&ims, tileX + i * dxt, tileY - 1, zoom);
			CopyTileToScreen(mapCache[index].data, xx + i * dxt * TILE_CX, yy - TILE_CY / 2, &screen);
		}
		if (yy + TILE_CY / 2 < SCREEN_CY)
		{
			index = cacheRead(&ims, tileX + i * dxt, tileY + 1, zoom);
			CopyTileToScreen(mapCache[index].data, xx + i * dxt * TILE_CX, yy + TILE_CY / 2, &screen);
		}
	}

	const int x = SCREEN_CX / 2;
	const int y = SCREEN_CY / 2;
	Line(x - 10, y, x + 10, y, DEV_RED, &screen);
	Line(x, y - 10, x, y + 10, DEV_RED, &screen);

	redrawScreen = true;
}

void Device::paint(const PaintContext* ctx)
{
	PaintScreen(ctx, &screen);
	redrawScreen = false;
}

void Device::run()
{
	while (gps.size())
	{
		processGps(gps.front());
		gps.pop_front();
	}
}
