#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "types.h"
#include "coord.h"
#include "sizes.h"

#include <deque>

struct IMS;

struct MapCacheItem
{
	ui8 zoom;
	ui32 tileX, tileY;
	ui8 data[TILE_CX * TILE_CY / 2];
};

class Device
{
	int id;

public:
	std::deque<PointFloat> gps;
	PointFloat currentTile;
	bool redrawScreen;

	ui8 screen[SCREEN_CX][SCREEN_CY/2];
	MapCacheItem mapCache[6];

	Device();
	Device(const Device&) = delete;
	void init(int id_);
	void run();

private:
	void processGps(PointFloat point);
	void copyTileToScreen(const void* tile, int x, int y);
	ui32 cacheRead(const IMS* ims, ui32 tileX, ui32 tileY, ui32 zoom);
};

#endif // !__DEVICE_H__
