#define _CRT_SECURE_NO_WARNINGS
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "resource.h"

#include "lodepng.h"
#include "convert.h"
#include "device.h"
#include "coord.h"
#include "graph.h"
#include "display.h"
#include "sizes.h"
#include "devio.h"
#include "devioimpl.h"

#include <stdlib.h>
#include <eh.h>
#include <assert.h>

static HINSTANCE hInst;          // current instance
static WCHAR szWindowClass[128]; // the main window class name

// Forward declarations of functions included in this code module:
static ATOM MyRegisterClass(HINSTANCE hInstance);
static BOOL InitInstance(HINSTANCE, int);
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

extern DeviceInput input;
extern "C" ui8 MapZoom;
extern "C" si8 MapShiftH;
extern "C" si8 MapShiftV;

static void __cdecl trans_func(unsigned int code, EXCEPTION_POINTERS *)
{
 Beep(440, 200);
 throw code;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
 UNREFERENCED_PARAMETER(hPrevInstance);
 UNREFERENCED_PARAMETER(lpCmdLine);

 _set_se_translator(trans_func);

 LoadStringW(hInstance, IDC_COMPRESSTEST, szWindowClass, sizeof(szWindowClass) / sizeof(*szWindowClass));
 MyRegisterClass(hInstance);

 if (!InitInstance(hInstance, nCmdShow))
  return FALSE;

 MSG msg;
 while (GetMessage(&msg, nullptr, 0, 0))
  {
   TranslateMessage(&msg);
   DispatchMessage(&msg);

   Run();
   if (NeedRedraw())
    InvalidateRect(msg.hwnd, NULL, FALSE);
  }
 return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
 WNDCLASSEXW wcex;

 wcex.cbSize = sizeof(WNDCLASSEX);

 wcex.style = 0; // CS_HREDRAW | CS_VREDRAW;
 wcex.lpfnWndProc = WndProc;
 wcex.cbClsExtra = 0;
 wcex.cbWndExtra = 0;
 wcex.hInstance = hInstance;
 wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COMPRESSTEST));
 wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
 wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
 wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_COMPRESSTEST);
 wcex.lpszClassName = szWindowClass;
 wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

 return RegisterClassExW(&wcex);
}

#pragma pack(push, 1)
struct CommMsg
{
 ui8 cmd;
 ui16 len;
 ui32 sector;
 ui8 data[512];
 ui8 chksum;
};
#pragma pack(pop)

static void transmit(HANDLE to, HANDLE from, ui32 sector)
{
 constexpr ui8 SD_WRITE_COMMAND = 0x12;
 constexpr ui8 SD_READ_COMMAND = 0x21;

 CommMsg buff;
 buff.cmd = SD_WRITE_COMMAND;
 buff.len = sizeof(buff.sector) + sizeof(buff.data);
 buff.sector = sector;
 buff.chksum = 0;

 DWORD fp = SetFilePointer(from, sector * sizeof(buff.data), NULL, FILE_BEGIN);
 DWORD n;
 if (!ReadFile(from, buff.data, sizeof(buff.data), &n, NULL) || (n != sizeof(buff.data)))
  throw "ERR";
 for (int i = 0; i < sizeof(buff) - 1; ++i)
  buff.chksum += *((ui8 *)&buff + i);
 WriteFile(to, &buff, sizeof(buff), &n, NULL);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
 hInst = hInstance; // Store instance handle in our global variable
#if 0
 HANDLE src = CreateFileA("SD0.BIN", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 HANDLE hCommPort = CreateFileA("COM2", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 DWORD sz = GetFileSize(src, NULL);
 for (ui32 i = 0; i < sz / BLOCK_SIZE; ++i)
  {
   transmit(hCommPort, src, i);
   const ui32 replyLen = 5;
   char s[replyLen];
   ui32 total = 0;
   do
    {
     DWORD n;
     if (!ReadFile(hCommPort, s + total, replyLen - total, &n, NULL))
      throw "READ ERR";
     total += n;
     Sleep(0);
    }
   while (total < replyLen);
   if (s[0] & 0x80)
    throw "DEVICE ERR";
   if (*(ui32 *)(s + 1) != i)
    throw "WRONG ADDR";
  }
 CloseHandle(hCommPort);
 CloseHandle(src);
#endif

 WCHAR s[64];
 wsprintf(s, L"Device %u", 0);
 HWND hWnd = CreateWindowW(szWindowClass, s, WS_OVERLAPPEDWINDOW,
                           0, 0, 240 + 64 + 20, 400 + 64 + 100, nullptr, nullptr, hInstance, nullptr);
 if (!hWnd)
  return FALSE;

 DeviceInit();

 SetTimer(hWnd, 1, 5000, NULL);
 ShowWindow(hWnd, nCmdShow);
 UpdateWindow(hWnd);

 return TRUE;
}

static PointFloat NextGps(WPARAM w)
{
 static PointFloat point = { 38.39f, 56.01f };
 //static PointFloat point = { -79.48f, 44.00f };

 if (VK_UP == w)
  point.y += 0.001f;
 else if (VK_DOWN == w)
  point.y -= 0.001f;
 else if (VK_LEFT == w)
  point.x -= 0.001f;
 else if (VK_RIGHT == w)
  point.x += 0.001f;
 return point;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
 switch (message)
  {
   case WM_COMMAND:
    {
     int wmId = LOWORD(wParam);
     // Parse the menu selections:
     switch (wmId)
      {
       case IDM_EXIT:
        DestroyWindow(hWnd);
        break;
       default:
        return DefWindowProc(hWnd, message, wParam, lParam);
      }
    }
    break;
   case WM_PAINT:
    {
     PAINTSTRUCT ps;
     HDC hdc = BeginPaint(hWnd, &ps);

     ScreenPaint();
     DisplayRedraw(hdc);
     ResetRedraw();

     EndPaint(hWnd, &ps);
    }
    break;
   case WM_CHAR:
    {
     if ('+' == wParam || '-' == wParam)
      {
       if ('+' == wParam)
        {
         if (MapZoom < MAX_ZOOM_LEVEL)
          MapZoom++;
        }
       else
        {
         if (MapZoom > MIN_ZOOM_LEVEL)
          MapZoom--;
        }
      }
     else if (VK_RETURN == wParam)
      {
       MapShiftH = MapShiftV = 0;
      }
     else if ('1' == wParam)
      {
       MapShiftH--;
      }
     else if ('2' == wParam)
      {
       MapShiftH++;
      }
     else if ('3' == wParam)
      {
       MapShiftV--;
      }
     else if ('4' == wParam)
      {
       MapShiftV++;
      }
    }
    break;
   case WM_KEYDOWN:
    {
     input.gps.push_back(NextGps(wParam));
    }
    break;
   case WM_TIMER:
    {
     CompassData c;
     c.x = rand() % 256 - 128;
     c.y = rand() % 256 - 128;
     c.z = rand() % 256 - 128;
     input.compass.push_back(c);
    }
    break;
   case WM_LBUTTONDOWN:
    {
     if (wParam == MK_LBUTTON)
      {
       int x = LOWORD(lParam);
       int y = HIWORD(lParam);
       ui8 b = TestButton(x, y);
       if (b)
        {
        }
      }
    }
    break;
   case WM_DESTROY:
    PostQuitMessage(0);
    break;
   default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
 return 0;
}
