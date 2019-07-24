#ifndef __sound_h
#define __sound_h
#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif
 void Sound(ui16 tick);
 void Vibro(ui8 num, ui16 tick);

#ifdef __cplusplus
}
#endif

#endif // __sound_h