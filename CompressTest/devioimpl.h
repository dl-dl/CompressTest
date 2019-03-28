#ifndef _DEVIOIMPL_H
#define _DEVIOIMPL_H
#include "types.h"
#include "coord.h"
#include "devio.h"
#include <deque>

typedef struct
{
 std::deque<PointFloat> gps;
 std::deque<CompassData> compass;
} DeviceInput;

#endif
