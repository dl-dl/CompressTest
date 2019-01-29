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
	zoom = 12;
	currentPoint = { 0, 0 };
	tileShift = { 0, 0 };
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

void Device::processKey(ui16 c)
{
	if (c == '+' && zoom < 13)
		zoom++;
	else if (c == '-' && zoom > 12)
		zoom--;

	screenToPoint();
}

void Device::screenToPoint()
{
	currentTile.x = lon2tilex(currentPoint.x, zoom);
	currentTile.y = lat2tiley(currentPoint.y, zoom);
	PointInt tileInt;
	tileInt.x = (int)currentTile.x;
	tileInt.y = (int)currentTile.y;
	PointInt tilePart;
	tilePart.x = (int)((currentTile.x - tileInt.x) * TILE_CX);
	tilePart.y = (int)((currentTile.y - tileInt.y) * TILE_CY);
	PointInt shift;
	shift.x = SCREEN_CX / 2 - tilePart.x;
	shift.y = SCREEN_CY / 2 - tilePart.y;
	if ((abs(tilePart.x + tileShift.x - SCREEN_CX / 2) > SCREEN_CX / 10)
		|| (abs(tilePart.y + tileShift.y - SCREEN_CY / 2)) > SCREEN_CY / 10)
		tileShift = shift;
#ifdef _DEBUG
	memset(&screen, 0, sizeof(screen));
#endif
	if (!PointInRect(&ims.coord, currentPoint.x, currentPoint.y))
		fsFindIMS(currentPoint.x, currentPoint.y, &ims, id);

	int dxt = 0;
	if (tileShift.x > 0)
		dxt = -1;
	else if (tileShift.x + TILE_CX < SCREEN_CX)
		dxt = 1;
	for (int i = 0; i <= abs(dxt); ++i)
	{
		ui32 index = cacheRead(&ims, tileInt.x + i * dxt, tileInt.y, zoom);
		CopyTileToScreen(mapCache[index].data, tileShift.x + i * dxt * TILE_CX, tileShift.y / 2, &screen);
		if (tileShift.y / 2 > 0)
		{
			index = cacheRead(&ims, tileInt.x + i * dxt, tileInt.y - 1, zoom);
			CopyTileToScreen(mapCache[index].data, tileShift.x + i * dxt * TILE_CX, (tileShift.y - TILE_CY) / 2, &screen);
		}
		if ((tileShift.y + TILE_CY) / 2 < SCREEN_CY)
		{
			index = cacheRead(&ims, tileInt.x + i * dxt, tileInt.y + 1, zoom);
			CopyTileToScreen(mapCache[index].data, tileShift.x + i * dxt * TILE_CX, (tileShift.y + TILE_CY) / 2, &screen);
		}
	}

	const int x = tilePart.x + tileShift.x;
	const int y = tilePart.y + tileShift.y;
	Line(x - 10, y, x + 10, y, DEV_RED, &screen);
	Line(x, y - 10, x, y + 10, DEV_RED, &screen);

	redrawScreen = true;
}

void Device::processGps(PointFloat point)
{
	currentPoint = point;

	screenToPoint();
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
	while (key.size())
	{
		processKey(key.front());
		key.pop_front();
	}
}
