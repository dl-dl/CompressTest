#include "device.h"
#include "fs.h"
#include "coord.h"
#include "convert.h"
#include "graph.h"
#include "text.h"
#include "paint.h"

#include <string.h>
#include <assert.h>

void Device::Init(int id_)
{
	for (int i = 0; i < sizeof(mapCache) / sizeof(*mapCache); ++i)
	{
		mapCache[i].zoom = 0; // empty
	}
	memset(&ims, 0, sizeof(ims));
	group.n = 0;

	timer = false;
	id = id_;
	zoom = 12;
	currentPos = { 0, 0 };
}

static PointInt PointFloat2Int(PointFloat pos, int zoom)
{
	PointInt p;
	p.x = (int)(lon2tilex(pos.x, zoom) * TILE_CX);
	p.y = (int)(lat2tiley(pos.y, zoom) * TILE_CY);
	return p;
}

static inline ui32 CacheIndex(ui32 x, ui32 y)
{
	return (x % 2) + (y % 3) * 2;
}

static inline bool CacheEq(const MapCacheItem* p, ui32 x, ui32 y, ui32 z)
{
	return (p->tileX == x) && (p->tileY == y) && (p->zoom == z);
}

ui32 Device::CacheRead(const IMS* ims, ui32 tileX, ui32 tileY, ui32 zoom)
{
	ui32 index = CacheIndex(tileX, tileY);
	if (!CacheEq(&mapCache[index], tileX, tileY, zoom))
	{
		auto addr = FsFindTile(ims, zoom, tileX, tileY, id);
		if (addr)
			FsReadTile(addr, mapCache[index].data, id);
		else
			memset(mapCache[index].data, 0xFF, sizeof(mapCache[0].data));
		mapCache[index].zoom = zoom;
		mapCache[index].tileX = tileX;
		mapCache[index].tileY = tileY;
	}
	return index;
}

void Device::ProcessKey(ui16 c)
{
	if (c == '+' && zoom < 14)
		zoom++;
	else if (c == '-' && zoom > 12)
		zoom--;

	screenStart.x = screenStart.y = 0x7FFFFFFF;
	redrawScreen = true;
}

void Device::AdjustScreenPos(PointInt pos)
{
	pos.x -= SCREEN_CX / 2;
	pos.y -= SCREEN_CY / 2;

	if ((abs(screenStart.x - pos.x) > SCREEN_CX / 8) ||
		(abs(screenStart.y - pos.y) > SCREEN_CY / 8))
	{
		screenStart = pos;
	}
}

void Device::DrawMap()
{
#ifdef _DEBUG
	memset(&screen, 0, sizeof(screen));
#endif
	if (!PointInRect(&ims.coord, currentPos.x, currentPos.y))
		FsFindIMS(currentPos.x, currentPos.y, &ims, id);

	for (int x = (screenStart.x / TILE_CX) * TILE_CX; x < screenStart.x + SCREEN_CX; x += TILE_CX)
		for (int y = (screenStart.y / TILE_CY) * TILE_CY; y < screenStart.y + SCREEN_CY; y += TILE_CY)
		{
			ui32 index = CacheRead(&ims, x / TILE_CX, y / TILE_CY, zoom);
			CopyTileToScreen(mapCache[index].data, x - screenStart.x, (y - screenStart.y) / 2, &screen);
		}
}

void Device::DrawGroup()
{
	for (int i = 0; i < group.n; ++i)
	{
		int dd = (group.data[i].id == id) ? 10 : 5;
		PointInt pos = group.data[i].pos;
		const int x = (pos.x >> (MAX_ZOOM_LEVEL - zoom)) - screenStart.x;
		const int y = (pos.y >> (MAX_ZOOM_LEVEL - zoom)) - screenStart.y;
		Line(x - dd, y, x + dd, y, DEV_RED, &screen);
		Line(x, y - dd, x, y + dd, DEV_RED, &screen);
	}
}

static int FindInGroup(const GroupData* g, int id)
{
	for (int i = 0; i < g->n; ++i)
	{
		if (g->data[i].id == id)
			return i;
	}
	return -1;
}

void Device::ProcessGps(PointFloat point)
{
	if (0 == memcmp(&point, &currentPos, sizeof(point)))
		return;
	currentPos = point;
	int i = FindInGroup(&group, id);
	const PointInt pos = PointFloat2Int(point, MAX_ZOOM_LEVEL);
	if (i >= 0)
	{
		group.data[i].pos = pos;
	}
	else
	{
		group.data[group.n].id = id;
		group.data[group.n].pos = pos;
		group.n++;
	}
	Broadcast(id, pos);
	redrawScreen = true;
}

void Device::ProcessRadio(const RadioMsg* msg)
{
	int i = FindInGroup(&group, msg->id);
	if (i >= 0)
		group.data[i] = *msg;
	else
		group.data[group.n++] = *msg;
	redrawScreen = true;
}

void Device::ProcessTimer()
{}

void Device::Paint(const PaintContext* ctx)
{
	PointInt pos = PointFloat2Int(currentPos, zoom);
	AdjustScreenPos(pos);
	DrawMap();
	DrawGroup();

	PaintScreen(ctx, &screen);
	redrawScreen = false;
}

void Device::Run()
{
	while (gps.size())
	{
		ProcessGps(gps.front());
		gps.pop_front();
	}
	while (radio.size())
	{
		ProcessRadio(&radio.front());
		radio.pop_front();
	}
	while (key.size())
	{
		ProcessKey(key.front());
		key.pop_front();
	}
	if (timer)
	{
		ProcessTimer();
		timer = false;
	}
}
