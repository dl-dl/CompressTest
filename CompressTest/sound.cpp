#include "types.h"
#include "sizes.h"
#ifdef _WINDOWS
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "sound.h"

void Sound(ui8 num, ui16 tick)
{
 Beep(440, tick);
}

void Vibro(ui8 num, ui16 tick)
{
 Beep(330*8, tick);
}

#endif // _WINDOWS
