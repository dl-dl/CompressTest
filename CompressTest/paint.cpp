#include "types.h"
#include "sizes.h"
#ifdef _WINDOWS
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include "paint.h"

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
    r.left = (SCREEN_CX - BORDERX) * j;
    r.right = r.left - BORDERX;
    r.top = SCREEN_CY * i / 4 - BORDERY * 2;
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
 HBITMAP memBM = CreateCompatibleBitmap(hdc, SCREEN_CX, SCREEN_CY);
 HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, (HGDIOBJ)memBM);

 for (int x = 0; x < SCREEN_CX; ++x)
  for (int y = 0; y < SCREEN_CY / 2; ++y)
   {
    ui8 c = screen.line[x].pix[y];
    SetPixelV(hdcMem, x, y * 2 + 1, TranslateColor(c));
    SetPixelV(hdcMem, x, y * 2, TranslateColor(c >> 4));
   }

 BitBlt(hdc, 0, 0, SCREEN_CX, SCREEN_CY, hdcMem, 0, 0, SRCCOPY);

 SelectObject(hdcMem, hbmOld);
 DeleteDC(hdcMem);
 DeleteObject(memBM);
}

#endif
