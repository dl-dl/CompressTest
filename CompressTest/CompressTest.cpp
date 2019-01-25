#define _CRT_SECURE_NO_WARNINGS
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "CompressTest.h"
#include "lodepng.h"
#include "convert.h"
#include "device.h"
#include "coord.h"

#include <stdlib.h>
#include <stdio.h>
#include <eh.h>

static HINSTANCE hInst;                                // current instance
static WCHAR szWindowClass[128];            // the main window class name

// Forward declarations of functions included in this code module:
static ATOM                MyRegisterClass(HINSTANCE hInstance);
static BOOL                InitInstance(HINSTANCE, int);
static LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

//static unsigned char* gBmp24;
//static unsigned char gBmp8[TILE_CX][TILE_CY];
//static unsigned char gBmp4[TILE_CX][TILE_CY / 2];
static const int NUM_DEV = 3;
static Device dev[NUM_DEV];

/*
static int test()
{
	FILE* f = fopen("..\\1275.4.png", "rb");
	if (NULL == f)
		return -1;
	fseek(f, 0, SEEK_END);
	size_t pngsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	unsigned char* png = (unsigned char*)malloc(pngsize);
	if (NULL == png)
		return -2;
	fread(png, 1, pngsize, f);
	fclose(f);

	unsigned width, height;
	BYTE* b;
	unsigned error = lodepng_decode_memory(&b, &width, &height, png, pngsize, LCT_RGB, 8);
	if (error)
		return -1;
	free(png);
	gBmp24 = (BYTE*)malloc(TILE_CX * TILE_CY * 3);
	Invert24(b, gBmp24);
//	Invert8(b, gBmp8);
	free(b);

//	Convert24To8Approx(gBmp24, gBmp8);
	Convert24To8(gBmp24, gBmp8);
//	Convert8To24(gBmp8, gBmp24);
//	Convert24To8(gBmp24, gBmp8);
	Convert8To4(gBmp8, gBmp4);
//	Convert4To24(gBmp4, gBmp24);
	BYTE* p = (BYTE*)malloc(TILE_CX * TILE_CY);
	ui32 sz = Compress4BitBuffer(gBmp4, p);
	// 10584, 9831
	DeCompress(p, gBmp4);
	free(p);

	return 0;
}
*/
static void paintBmp(HWND hWnd, HDC hdc)
{
/*	if (!gBmp24)
		return;

	for (int x = 0; x < TILE_CX; ++x)
		for (int y = 0; y < TILE_CY; ++y)
		{
			const BYTE* p = gBmp24 + (x * TILE_CY + y) * 3;
			SetPixel(hdc, x, y, RGB(*p, *(p + 1), *(p + 2)));
		}

	for (int x = 0; x < TILE_CX; ++x)
		for (int y = 0; y < TILE_CY; ++y)
		{
			static const BYTE flag[3] = { 0x01, 0x02, 0x04 };
			BYTE b = gBmp8[x][y];
			SetPixel(hdc, x + (TILE_CX + 10), y, RGB((b & flag[0]) ? 0xFF : 0, (b & flag[1]) ? 0xFF : 0, (b & flag[2]) ? 0xFF : 0));
		}
*/
	SetWindowOrgEx(hdc, -32, -32, NULL);

	Device* devPtr = (Device*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	for (int x = 0; x < SCREEN_CX; ++x)
		for (int y = 0; y < SCREEN_CY / 2; ++y)
		{
			static const BYTE flag[6] = { 0x02, 0x04, 0x08, 0x20, 0x40, 0x80 };
			BYTE b = devPtr->screen[x][y];
			SetPixel(hdc, x, y * 2 + 1, RGB((b & flag[0]) ? 0xFF : 0, (b & flag[1]) ? 0xFF : 0, (b & flag[2]) ? 0xFF : 0));
			SetPixel(hdc, x, y * 2, RGB((b & flag[3]) ? 0xFF : 0, (b & flag[4]) ? 0xFF : 0, (b & flag[5]) ? 0xFF : 0));
		}

//	int x = (int)((dev.currentTilePos.x - floor(dev.currentTilePos.x)) * TILE_CX);
//	int y = (int)((dev.currentTilePos.y - floor(dev.currentTilePos.y)) * TILE_CY);
	int x = SCREEN_CX / 2;
	int y = SCREEN_CY / 2;

	auto pen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
	SelectObject(hdc, pen);
	MoveToEx(hdc, x - 10, y, NULL);
	LineTo(hdc, x + 10, y);
	MoveToEx(hdc, x, y - 10, NULL);
	LineTo(hdc, x, y + 10);
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	DeleteObject(pen);

	devPtr->redrawScreen = false;
}

static void __cdecl trans_func(unsigned int code, EXCEPTION_POINTERS*)
{
	throw code;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					  _In_opt_ HINSTANCE hPrevInstance,
					  _In_ LPWSTR    lpCmdLine,
					  _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	_set_se_translator(trans_func);

	// Initialize global strings
	LoadStringW(hInstance, IDC_COMPRESSTEST, szWindowClass, sizeof(szWindowClass) / sizeof(*szWindowClass));
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COMPRESSTEST));

//	test();
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.hwnd)
			{
				Device* devPtr = (Device*)GetWindowLongPtr(msg.hwnd, GWLP_USERDATA);
				if (devPtr)
				{
					devPtr->run();
					if (devPtr->redrawScreen)
						InvalidateRect(msg.hwnd, NULL, FALSE);
				}
			}
		}
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

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	for (int i = 0; i < 3; ++i)
	{
		WCHAR s[64];
		wsprintf(s, L"Device %u", i);
		HWND hWnd = CreateWindowW(szWindowClass, s, WS_OVERLAPPEDWINDOW,
								  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
		if (!hWnd)
			return FALSE;

		dev[i].init(i);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(&dev[i]));
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);
	}
	return TRUE;
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
		paintBmp(hWnd, hdc);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_KEYDOWN:
	{
		static PointFloat point = { 38.39f, 56.01f };
		if (VK_UP == wParam)
			point.y += 0.01f;
		else if (VK_DOWN == wParam)
			point.y -= 0.01f;
		else if (VK_LEFT == wParam)
			point.x -= 0.01f;
		else if (VK_RIGHT == wParam)
			point.x += 0.01f;
		else
			break;
		Device* devPtr = (Device*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		devPtr->gps.push_back(point);
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
