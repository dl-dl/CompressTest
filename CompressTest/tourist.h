#ifndef __tourist_h
#define __tourist_h

#include "types.h"

typedef struct
{
 ui8   IsRx;
 ui8   Flags;
 ui8   RxRSSI;
 ui8   TxRSSI;
 si32  FreqError;
 ui32  TimeLastRx;
 
 float CoordX;
 float CoordY;
 ui32  CoordTileX;
 ui32  CoordTileY;
 
 ui8   IsOld;
 ui8   MessageId;
 ui8   Power;
 ui8   Rezerv;
 char  Name[8];
}  TTourist;

#endif /* __tourist_h */
