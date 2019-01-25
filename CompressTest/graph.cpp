#define _CRT_SECURE_NO_WARNINGS
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "types.h"
#include "graph.h"
#include "paintctx.h"

static const int BORDERX = -32;
static const int BORDERY = -32;

static inline COLORREF translateColor(ui8 c)
{
	return RGB((c & DEV_RED) ? 0xFF : 0, (c & DEV_GREEN) ? 0xFF : 0, (c & DEV_BLUE) ? 0xFF : 0);
}

void gPixel(const PaintContext* ctx, int x, int y, ui8 color)
{
	SetWindowOrgEx(ctx->hdc, BORDERX, BORDERY, NULL);

	SetPixel(ctx->hdc, x, y, translateColor(color));
}

void gLine(const PaintContext* ctx, int x0, int y0, int x1, int y1, int color)
{
	SetWindowOrgEx(ctx->hdc, BORDERX, BORDERY, NULL);

	auto pen = CreatePen(PS_SOLID, 0, translateColor(color));
	SelectObject(ctx->hdc, pen);
	MoveToEx(ctx->hdc, x0, y0, NULL);
	LineTo(ctx->hdc, x1, y1);
	SelectObject(ctx->hdc, GetStockObject(BLACK_PEN));
	DeleteObject(pen);
}

void gCircle(const PaintContext* ctx, int x, int y, int r, ui8 color)
{
	SetWindowOrgEx(ctx->hdc, BORDERX, BORDERY, NULL);

	auto pen = CreatePen(PS_SOLID, 0, translateColor(color));
	SelectObject(ctx->hdc, pen);
	SelectObject(ctx->hdc, GetStockObject(HOLLOW_BRUSH));
	Ellipse(ctx->hdc, x - r, y - r, x + r, y + r);
	SelectObject(ctx->hdc, GetStockObject(BLACK_PEN));
	DeleteObject(pen);
}

void gText(const PaintContext* ctx, int x, int y, ui8 color)
{
	SetWindowOrgEx(ctx->hdc, BORDERX, BORDERY, NULL);

}
