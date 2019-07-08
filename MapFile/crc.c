#include "types.h"

ui32 MapCalcCRC(const void *data, ui32 sz) // TODO: implement proper algorithm
{
 ui32 crc = 0;
 for (ui32 i = 0; i < sz; ++i)
  crc += *((ui8 *)data + i);
 return crc;
}
