#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "types.h"
#include "coord.h"
#include "sizes.h"
#include "fs.h"
#include "fscache.h"
#include "screen.h"

#include <deque> // TODO: remove

struct RadioMsg
{
	PointInt pos;
	ui32 id;
};

struct CompassData
{
	int x, y, z;
};

struct GroupData
{
	RadioMsg data[16];
	ui32 n;
};

class Device
{
	FsMapCache FsCache;
	PointInt currentPos;
	PointInt screenStart;
	ui8 zoom;
	GroupData group;
	CompassData currentCompass;

public:
	int deviceId;
	mutable Screen screen;
	bool redrawScreen;
	std::deque<PointFloat> gps;
	std::deque<RadioMsg> radio;
	std::deque<ui16> key;
	std::deque<CompassData> compass;
	std::deque<ui8> button;
	bool timer;

	Device()
	{}
	Device(const Device&) = delete;

	void Init(int id_);
	void Run();
	void Paint();

private:
	void AdjustScreenPos(PointInt pos);
	void DrawMap(void);
	void DrawGroup(void);
	void DrawCompass(void);

	void ProcessKey(ui16 c);
	void ProcessGps(PointFloat point);
	void ProcessRadio(const RadioMsg* point);
	void ProcessTimer(void);
	void ProcessCompass(CompassData d);
	void ProcessButton(ui8 b);
};

extern void Broadcast(int srcId, PointInt data);

#endif // !__DEVICE_H__
