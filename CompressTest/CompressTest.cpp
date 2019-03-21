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
#include "paint.h"
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

static HWND wnd[NUM_DEV];

extern DeviceInput input[NUM_DEV];
struct Device;
extern "C" Device dev[NUM_DEV];

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

   for (int i = 0; i < NUM_DEV; ++i)
    Run(i);
   for (int i = 0; i < NUM_DEV; ++i)
    if (NeedRedraw(i))
     InvalidateRect(wnd[i], NULL, FALSE);
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
 ui8 data[BLOCK_SIZE];
 ui8 chksum;
};
#pragma pack(pop)

static void transmit(HANDLE to, HANDLE from, ui32 sector)
{
 CommMsg buff;
 buff.cmd = 0;
 buff.len = sizeof(buff.sector) + sizeof(buff.data);
 buff.sector = sector;
 buff.chksum = 0;

 DWORD fp = SetFilePointer(from, sector * BLOCK_SIZE, NULL, FILE_BEGIN);
 DWORD n;
 ReadFile(from, buff.data, BLOCK_SIZE, &n, NULL);
 if (n != BLOCK_SIZE)
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
   char s[8];
   ui32 total = 0;
   do
    {
     DWORD n;
     ReadFile(hCommPort, s + total, 2 - total, &n, NULL);
     total += n;
     Sleep(0);
    }
   while (total < 2);
   if (memcmp(s, "Ok", 2))
    throw "ERR";
  }
 CloseHandle(hCommPort);
 CloseHandle(src);
#endif

 for (int i = 0; i < NUM_DEV; ++i)
  {
   WCHAR s[64];
   wsprintf(s, L"Device %u", i);
   HWND hWnd = CreateWindowW(szWindowClass, s, WS_OVERLAPPEDWINDOW,
                             (240 + 64 + 20) * i, 0, 240 + 64 + 20, 400 + 64 + 100, nullptr, nullptr, hInstance, nullptr);
   if (!hWnd)
    return FALSE;

   wnd[i] = hWnd;
   DeviceInit(i);
   SetTimer(hWnd, 1, 5000, NULL);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
  }
 return TRUE;
}

static PointFloat NextGps(int id, WPARAM w)
{
 //static PointFloat point[NUM_DEV] = { { 38.39f, 56.01f }, { 38.39f, 56.01f }, { 38.39f, 56.01f } };
 static PointFloat point[NUM_DEV] = { { -79.48f, 44.00f }, { -79.48f, 44.00f }, { -79.48f, 44.00f } };

 if (VK_UP == w)
  point[id].y += 0.001f;
 else if (VK_DOWN == w)
  point[id].y -= 0.001f;
 else if (VK_LEFT == w)
  point[id].x -= 0.001f;
 else if (VK_RIGHT == w)
  point[id].x += 0.001f;
 return point[id];
}

static int getDevice(HWND hwnd)
{
 for (int i = 0; i < NUM_DEV; ++i)
  if (hwnd == wnd[i])
   return i;
 assert(0);
 return 0;
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

     int id = getDevice(hWnd);
     ScreenPaint(id);
     DisplayRedraw(hdc, GetScreen(id));
     ResetRedraw(id);

     EndPaint(hWnd, &ps);
    }
    break;
   case WM_CHAR:
    {
     if ('+' == wParam || '-' == wParam)
      {
       int id = getDevice(hWnd);
       input[id].button.push_back(('-' == wParam) ? 0x80 : 0x20);
      }
    }
    break;
   case WM_KEYDOWN:
    {
     int id = getDevice(hWnd);
     input[id].gps.push_back(NextGps(id, wParam));
    }
    break;
   case WM_TIMER:
    {
     CompassData c;
     c.x = rand() % 256 - 128;
     c.y = rand() % 256 - 128;
     c.z = rand() % 256 - 128;
     int id = getDevice(hWnd);
     input[id].compass.push_back(c);
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
         int id = getDevice(hWnd);
         input[id].button.push_back(b);
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
