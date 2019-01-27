#include "device.h"
#include "fs.h"
#include "coord.h"
#include "convert.h"
#include "graph.h"
#include "text.h"
#include "paint.h"

#include <string.h>
#include <assert.h>

void Device::init(int id_)
{
	for (int i = 0; i < sizeof(mapCache) / sizeof(*mapCache); ++i)
	{
		mapCache[i].zoom = 0; // empty
	}
	memset(&ims, 0, sizeof(ims));

	id = id_;
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
			memset(mapCache[index].data, 0xFF, TILE_CX * TILE_CY / 2);
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

	int xx = (int)((currentTile.x - tileX) * TILE_CX); // screen offset x
	xx = SCREEN_CX / 2 - xx;
	int yy = (int)((currentTile.y - tileY) * TILE_CY); // screen offset y
	yy = SCREEN_CY / 2 - yy;
	yy /= 2;
#ifdef _DEBUG
	memset(&screen, 0, sizeof(screen));
#endif
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
