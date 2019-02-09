#include "types.h"
#include "sizes.h"
#include "paint.h"
#ifdef _WINDOWS
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#define NOUSER
#include <windows.h>
#include "paintctx.h"

static const int BORDERX = -32;
static const int BORDERY = -32;

static inline COLORREF translateColor(ui8 c)
{
	return RGB((c & DEV_RED) ? 0xFF : 0, (c & DEV_GREEN) ? 0xFF : 0, (c & DEV_BLUE) ? 0xFF : 0);
}

void PaintScreen(const PaintContext* ctx, const Screen* screen)
{
	SetWindowOrgEx(ctx->hdc, BORDERX, BORDERY, NULL);

	HDC hdcMem = CreateCompatibleDC(ctx->hdc);
	HBITMAP memBM = CreateCompatibleBitmap(ctx->hdc, SCREEN_CX, SCREEN_CY);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, (HGDIOBJ)memBM);

	for (int x = 0; x < SCREEN_CX; ++x)
		for (int y = 0; y < SCREEN_CY / 2; ++y)
		{
			ui8 c = screen->line[x].pix[y];
			SetPixelV(hdcMem, x, y * 2 + 1, translateColor(c));
			SetPixelV(hdcMem, x, y * 2, translateColor(c >> 4));
		}

	BitBlt(ctx->hdc, 0, 0, SCREEN_CX, SCREEN_CY, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);
	DeleteObject(memBM);
}

#endif
