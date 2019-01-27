#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "types.h"
#include "coord.h"
#include "sizes.h"
#include "fs.h"
#include "screen.h"

#include <deque>

struct PaintContext;

struct MapCacheItem
{
	ui8 zoom;
	ui32 tileX, tileY;
	ui8 data[TILE_CX * TILE_CY / 2];
};

class Device
{
	int id;

	IMS ims;
	MapCacheItem mapCache[6];
	PointFloat currentTile;
	Screen screen;

public:
	bool redrawScreen;
	std::deque<PointFloat> gps;

	Device()
	{}
	Device(const Device&) = delete;

	void init(int id_);
	void run();
	void paint(const PaintContext* ctx);

private:
	void processGps(PointFloat point);
	ui32 cacheRead(const IMS* ims, ui32 tileX, ui32 tileY, ui32 zoom);
};

#endif // !__DEVICE_H__
