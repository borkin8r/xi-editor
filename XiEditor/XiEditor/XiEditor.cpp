// Xi-Editor.cpp : Defines the entry point for the application.
//

#include "XiEditor.h"

LRESULT CALLBACK WindowCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int CALLBACK
WinMain(HINSTANCE instance,
HINSTANCE prevInstance,
LPTSTR    commandLine,
int       cmdShow)
{
	// Register the window class.
	WNDCLASSA windowClass = {};

	windowClass.lpfnWndProc = WindowCallback;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = "Xi-Editor";

	if (RegisterClassA(&windowClass))
	{
		HWND hwnd = CreateWindowExA(
			0,                              // Optional window styles.
			windowClass.lpszClassName,                     // Window class
			"Xi-Editor text",     // Window text
			WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Window style
			// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL,       // Parent window    
			NULL,       // Menu
			instance,  // Instance handle
			NULL        // Additional application data
			);

		if (hwnd == NULL)
		{
			return 0;
		}

		ShowWindow(hwnd, cmdShow);

		// Run the message loop.

		MSG msg = {};
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return 0;
	}


	return(0);
}

LRESULT CALLBACK
WindowCallback(HWND hwnd,
UINT msg,
WPARAM wParam,
LPARAM lParam) {

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(hwnd, &ps);
	}
	return 0;

	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
