#include "types.h"
#include "sizes.h"
#include "paint.h"
#ifdef _WINDOWS
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

void PaintScreen(const PaintContext* ctx, Screen* screen)
{
	SetWindowOrgEx(ctx->hdc, BORDERX, BORDERY, NULL);
	for (int x = 0; x < SCREEN_CX; ++x)
		for (int y = 0; y < SCREEN_CY / 2; ++y)
		{
			ui8 c = screen->line[x].pix[y];
			SetPixelV(ctx->hdc, x, y * 2 + 1, translateColor(c));
			SetPixelV(ctx->hdc, x, y * 2, translateColor(c >> 4));
		}
}

#endif
