#ifndef __LOGBUFF_H__
#define __LOGBUFF_H__

#include "types.h"

typedef struct
{
 ui16 OkCnt;      //2  bytes
 ui8 RSSI;        //1  bytes
 ui8 TxLevel;     //1  bytes
 char Time[8];    //8  bytes
 float RemCoordX; //4  bytes
 float RemCoordY; //4  bytes
 float LocCoordX; //4  bytes
 float LocCoordY; //4  bytes
 float Vakk;      //4  bytes
} TLogBuff;       //32 bytes

#define LOG_LEN 1600

extern ui8 LogOn;
extern TLogBuff LogBuff[LOG_LEN]; //32*1600 = 51 200 = 512 * 100;
extern ui16 LogCnt;

#endif // !__LOGBUFF_H__
