#pragma once

struct PointFloat
{
	float x, y;
};

struct RectFloat
{
	float left, top, right, bottom;
};

bool PointInRect(const RectFloat* r, float x, float y);

float lon2tilex(float lon, unsigned int z);
float lat2tiley(float lat, unsigned int z);

