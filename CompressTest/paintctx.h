#ifndef __PAINTCTX_H_
#define __PAINTCTX_H_
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

struct PaintContext
{
	HDC hdc;
};

#endif
#endif
