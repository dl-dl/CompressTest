#ifndef __CONVERT_H
#define __CONVERT_H

#include "types.h"

typedef struct
{
 ui8 *dst;
 int x, y;
 bool eqString;
 ui8 numEqPixel;
} DecompState;

#ifdef __cplusplus
extern "C"
{
#endif
 void DecompImit(DecompState *s, ui8 *dst);
 bool DeCompressOne(ui8 src, DecompState *s);
#ifdef __cplusplus
}
#endif

#endif
