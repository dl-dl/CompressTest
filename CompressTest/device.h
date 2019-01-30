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
	IMS ims;
	MapCacheItem mapCache[6];
	PointFloat currentPoint;
	PointFloat currentTile;
	PointInt tileShift;
	ui8 zoom;
	Screen screen;

public:
	int id;
	bool redrawScreen;
	std::deque<PointFloat> gps;
	std::deque<ui16> key;

	Device()
	{}
	Device(const Device&) = delete;

	void init(int id_);
	void run();
	void paint(const PaintContext* ctx);

private:
	void screenToPoint();
	void processKey(ui16 c);
	void processGps(PointFloat point);
	ui32 cacheRead(const IMS* ims, ui32 tileX, ui32 tileY, ui32 zoom);
};

#endif // !__DEVICE_H__
