#define _CRT_SECURE_NO_WARNINGS
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "resource.h"

#include "lodepng.h"
#include "convert.h"
#include "device.h"
#include "fsinit.h"
#include "coord.h"
#include "graph.h"
#include "paintctx.h"

#include <stdlib.h>
#include <eh.h>

static HINSTANCE hInst;                                // current instance
static WCHAR szWindowClass[128];            // the main window class name

// Forward declarations of functions included in this code module:
static ATOM                MyRegisterClass(HINSTANCE hInstance);
static BOOL                InitInstance(HINSTANCE, int);
static LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

static const int NUM_DEV = 3;
static Device dev[NUM_DEV];

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

	LoadStringW(hInstance, IDC_COMPRESSTEST, szWindowClass, sizeof(szWindowClass) / sizeof(*szWindowClass));
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
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

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	for (int i = 0; i < 3; ++i)
	{
		WCHAR s[64];
		wsprintf(s, L"Device %u", i);
		HWND hWnd = CreateWindowW(szWindowClass, s, WS_OVERLAPPEDWINDOW,
								  CW_USEDEFAULT, 0, 240 + 64 + 20, 400 + 64 + 100, nullptr, nullptr, hInstance, nullptr);
		if (!hWnd)
			return FALSE;

		fsFormat(i);
		fsInit(i);

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
		PaintContext ctx;
		ctx.hdc = hdc;
		Device* devPtr = (Device*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		devPtr->paint(&ctx);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CHAR:
	{
		Device* devPtr = (Device*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if ('+' == wParam || '-' == wParam)
			devPtr->key.push_back(wParam);
	}
	break;
	case WM_KEYDOWN:
	{
		Device* devPtr = (Device*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		static PointFloat point = { 38.39f, 56.01f };
		if (VK_UP == wParam)
			point.y += 0.001f;
		else if (VK_DOWN == wParam)
			point.y -= 0.001f;
		else if (VK_LEFT == wParam)
			point.x -= 0.001f;
		else if (VK_RIGHT == wParam)
			point.x += 0.001f;
		else
			break;
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
