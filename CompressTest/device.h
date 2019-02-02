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
	PointFloat currentPos;
	PointInt screenStart;
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

	void Init(int id_);
	void Run();
	void Paint(const PaintContext* ctx);

private:
	void AdjustScreenPos(PointInt pos);
	void DrawMap();
	void DrawGroup();

	void ProcessKey(ui16 c);
	void ProcessGps(PointFloat point);
	void ProcessRadio(PadioMsg point);
	void ProcessTimer();

	ui32 CacheRead(const IMS* ims, ui32 tileX, ui32 tileY, ui32 zoom);
};

extern void Broadcast(int srcId, PointFloat msg);

#endif // !__DEVICE_H__
