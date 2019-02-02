#ifndef __PAINTCTX_H_
#define __PAINTCTX_H_
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

struct PaintContext
{
	HDC hdc;
};

#endif
#endif
