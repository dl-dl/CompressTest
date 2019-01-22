#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "types.h"
#include "coord.h"

#include <deque>

#define SCREEN_CX 240
#define SCREEN_CY 400

class Device
{
	int id;

public:
	std::deque<PointFloat> gps;
	PointFloat currentTilePos;
	bool redrawScreen;

	ui8 screen[SCREEN_CX][SCREEN_CY/2];
	ui8* map[4];

	Device(int id_);
	Device(const Device&) = delete;
	void init();
	void run();

private:
	void processGps(PointFloat point);
};

#endif // !__DEVICE_H__
