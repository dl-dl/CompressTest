#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "types.h"
#include "coord.h"
#include "sizes.h"
#include "fs.h"
#include "screen.h"

#include <deque>
#include <map>

struct PaintContext;

struct MapCacheItem
{
	ui8 zoom;
	ui32 tileX, tileY;
	ui8 data[TILE_CX * TILE_CY / 2];
};

struct PadioMsg
{
	PointFloat pos;
	ui32 id;
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
	std::deque<PadioMsg> radio;
	std::deque<ui16> key;
	bool timer;
	std::map<ui32, PointFloat> groupPos;

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
	void processRadio(PadioMsg point);
	void processTimer();

	ui32 cacheRead(const IMS* ims, ui32 tileX, ui32 tileY, ui32 zoom);
};

extern void broadcast(int srcId, PointFloat msg);

#endif // !__DEVICE_H__
