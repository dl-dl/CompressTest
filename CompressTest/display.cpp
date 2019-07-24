#include "types.h"
#include "sizes.h"
#ifdef _WINDOWS
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include "display.h"
#include "color.h"

extern "C"
{
 ui8 Screen[256][256];
 ui8 ScreenChange[256];
}

static const int BORDERX = -32;
static const int BORDERY = -32;

static std::vector<RECT> buttons;

static void InitButtons()
{
 if (buttons.size())
  return;
 for (int i = 0; i < 4; ++i)
  for (int j = 0; j < 2; ++j)
   {
    RECT r;
    r.left = (SCREEN_DX - BORDERX) * j;
    r.right = r.left - BORDERX;
    r.top = SCREEN_DY * i / 4 - BORDERY * 2;
    r.bottom = r.top - BORDERY;
    buttons.push_back(r);
   }
}

ui8 TestButton(int x, int y)
{
 POINT p = { x, y };
 for (size_t i = 0; i < buttons.size(); ++i)
  {
   if (PtInRect(&buttons[i], p))
    return 1 << i;
  }
 return 0;
}

static void DrawButtons(HDC hdc)
{
 for (size_t i = 0; i < buttons.size(); ++i)
  FillRect(hdc, &buttons[i], (HBRUSH)GetStockObject(GRAY_BRUSH));
}

#define DEV_RED 0x08
#define DEV_GREEN 0x04
#define DEV_BLUE 0x02

static inline COLORREF TranslateColor(ui8 c)
{
 return RGB((c & DEV_RED) ? 0xFF : 0, (c & DEV_GREEN) ? 0xFF : 0, (c & DEV_BLUE) ? 0xFF : 0);
}

void DisplayRedraw(HDC hdc)
{
 InitButtons();
 DrawButtons(hdc);

 SetWindowOrgEx(hdc, BORDERX, BORDERY, NULL);
 HDC hdcMem = CreateCompatibleDC(hdc);
 HBITMAP memBM = CreateCompatibleBitmap(hdc, SCREEN_DX, SCREEN_DY);
 HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, (HGDIOBJ)memBM);

 for (int x = 0; x < SCREEN_DX; ++x)
  for (int y = 0; y < SCREEN_DY / 2; ++y)
   {
    ui8 c = Screen[x][SCREEN_DY / 2 - 1 - y];
    SetPixelV(hdcMem, x, y * 2, TranslateColor(c & 0x0F));
    SetPixelV(hdcMem, x, y * 2 + 1, TranslateColor(c >> 4));
   }

 BitBlt(hdc, 0, 0, SCREEN_DX, SCREEN_DY, hdcMem, 0, 0, SRCCOPY);

 SelectObject(hdcMem, hbmOld);
 DeleteDC(hdcMem);
 DeleteObject(memBM);
}

#endif
