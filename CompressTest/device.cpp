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
#ifdef CREATE_NEW_SD
	FsFormat(id_);
	FsInit(id_);
#endif
//	BlockAddr sz = FsFreeSpace(id_);
	CacheInit(&FsCache, id_);

	group.n = 0;
	timer = false;
	deviceId = id_; // TODO: replace with hardware device id
	zoom = 12;
	currentPos = { 0x7FFFFFFF, 0x7FFFFFFF };
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
	pos.x = ScaleDownCoord(pos.x, zoom);
	pos.y = ScaleDownCoord(pos.y, zoom);
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
	PointInt pos;
	pos.x = currentPos.x / TILE_CX;
	pos.y = currentPos.y / TILE_CY;
	if (!CacheFetchIMS(&FsCache, pos.x, pos.y))
		return;

	for (int x = (screenStart.x / TILE_CX) * TILE_CX; x < screenStart.x + SCREEN_CX; x += TILE_CX)
		for (int y = (screenStart.y / TILE_CY) * TILE_CY; y < screenStart.y + SCREEN_CY; y += TILE_CY)
		{
			ui32 index = CacheRead(&FsCache, x / TILE_CX, y / TILE_CY, zoom);
			CopyTileToScreen(FsCache.map[index].data, x - screenStart.x, (y - screenStart.y) / 2, &screen);
		}
}

void Device::DrawGroup()
{
	for (ui32 i = 0; i < group.n; ++i)
	{
		PointInt pos = group.data[i].pos;
		int x = ScaleDownCoord(pos.x, zoom) - screenStart.x;
		int y = ScaleDownCoord(pos.y, zoom) - screenStart.y;
		int dd = (group.data[i].id == deviceId) ? 10 : 5;
		Line(x - dd, y, x + dd, y, DEV_RED, &screen);
		Line(x, y - dd, x, y + dd, DEV_RED, &screen);
	}
}

void Device::DrawCompass()
{
	const int COMPASS_POS_X = 20;
	const int COMPASS_POS_Y = 20;
	const int COMPASS_R = 16;
//	const int COMPASS_POS_X = 120;
//	const int COMPASS_POS_Y = 200;
//	const int COMPASS_R = COMPASS_POS_X - 8;

	Circle(COMPASS_POS_X, COMPASS_POS_Y, COMPASS_R, DEV_RED, &screen);
	const ui32 MIN2_COMPASS = 0x800; // TODO: find reasonable value
	const ui32 MIN3_COMPASS = 0x10000; // TODO: find reasonable value
	CompassData d = currentCompass;
	ui32 r2 = d.x * d.x + d.y * d.y;
	ui32 r3 = r2 + d.z * d.z;
	if (r3 > MIN3_COMPASS && r2 > MIN2_COMPASS)
	{
		float q = (float)COMPASS_R / sqrtf((float)r2);
		d.x = (ui32)(d.x * q);
		d.y = (ui32)(d.y * q);
	}
	else
	{
		return;
	}

	Line(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y,
		 COMPASS_POS_X + d.x, COMPASS_POS_Y + d.y, DEV_RED, &screen);
	Circle(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y, COMPASS_R / 8, DEV_RED, &screen);
	Circle(COMPASS_POS_X - d.x, COMPASS_POS_Y - d.y, COMPASS_R / 8 + 1, DEV_RED | DEV_GREEN | DEV_BLUE, &screen);
}

static int FindInGroup(const GroupData* g, int id)
{
	for (ui32 i = 0; i < g->n; ++i)
	{
		if (g->data[i].id == id)
			return i;
	}
	return -1;
}

void Device::ProcessGps(PointFloat fpoint)
{
	PointInt pos;
	pos.x = lon2tilex(fpoint.x, MAX_ZOOM_LEVEL);
	pos.y = lat2tiley(fpoint.y, MAX_ZOOM_LEVEL);
	if (PointEqInt(&pos, &currentPos))
		return;
	currentPos = pos;
	int i = FindInGroup(&group, deviceId);
	if (i >= 0)
	{
		group.data[i].pos = pos;
	}
	else
	{
		group.data[group.n].id = deviceId;
		group.data[group.n].pos = pos;
		group.n++;
	}
	Broadcast(deviceId, pos);
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

void Device::ProcessCompass(CompassData d)
{
	currentCompass = d;
	redrawScreen = true;
}

void Device::Paint(const PaintContext* ctx)
{
	AdjustScreenPos(currentPos);
	DrawMap();
	DrawCompass();
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
	while (compass.size())
	{
		ProcessCompass(compass.front());
		compass.pop_front();
	}
	if (timer)
	{
		ProcessTimer();
		timer = false;
	}
}
