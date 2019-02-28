#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "types.h"
#include "coord.h"
#include "sizes.h"
#include "fs.h"
#include "fscache.h"
#include "screen.h"
#include "devio.h"

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

	void ProcessGps(PointFloat point);
	void ProcessRadio(const RadioMsg* point);
	void ProcessTimer(void);
	void ProcessCompass(CompassData d);
	void ProcessButton(ui8 b);
};

#endif // !__DEVICE_H__
