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
	///////////////////////////////
	// Register the window class.

	WNDCLASSA windowClass = {};

	windowClass.lpfnWndProc = WindowCallback;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = "Xi-Editor";

	if (RegisterClassA(&windowClass))
	{
		HWND window = CreateWindowExA(
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

		if (window == NULL)
		{
			return 0;
		}

		ShowWindow(window, cmdShow);

		///////////////////////////////
		//Setup connection to Xi-Core

		//TODO: zero memory?
		PROCESS_INFORMATION procInfo = { 0 };
		STARTUPINFO startInfo = { 0 };
		HANDLE coreInputWriteEnd = NULL;
		HANDLE coreInputReadEnd = NULL;
		HANDLE coreOutputWriteEnd = NULL;
		HANDLE coreOutputReadEnd = NULL;
		SECURITY_ATTRIBUTES securityAttributes;

		securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		securityAttributes.bInheritHandle = TRUE;
		securityAttributes.lpSecurityDescriptor = NULL;


		if (!CreatePipe(&coreOutputReadEnd, &coreOutputWriteEnd, &securityAttributes, 0))
		{
			OutputDebugStringA("failed to create output pipe\n");
			return -1;
		}

		if (!SetHandleInformation(coreOutputReadEnd, HANDLE_FLAG_INHERIT, 0))
		{
			OutputDebugStringA("failed to lose read inheritance from parent for core output\n");
			return -1;
		}

		if (!CreatePipe(&coreInputReadEnd, &coreInputWriteEnd, &securityAttributes, 0))
		{
			OutputDebugStringA("failed to create input pipe\n");
			return -1;
		}

		if (!SetHandleInformation(coreInputWriteEnd, HANDLE_FLAG_INHERIT, 0))
		{
			OutputDebugStringA("failed to lose write inheritance from parent for core input\n");
			return -1;
		}

		startInfo.cb = sizeof(STARTUPINFO);
		startInfo.hStdError = coreOutputWriteEnd;
		startInfo.hStdOutput = coreOutputWriteEnd;
		startInfo.hStdInput = coreInputReadEnd;
		startInfo.dwFlags = STARTF_USESTDHANDLES;

		///////////////////////////
		// create core connection

		BOOL coreProcessStarted = FALSE;

		coreProcessStarted = CreateProcess(
			NULL,		   //path to executable "xi-core",
			"dir",		   // command line arguements
			NULL,          // process security attributes 
			NULL,          // primary thread security attributes 
			TRUE,          // handles are inherited 
			0,             // creation flags 
			NULL,          // use parent's environment 
			NULL,          // use parent's current directory 
			&startInfo,    // STARTUPINFO pointer 
			&procInfo);    // receives PROCESS_INFORMATION

		if (!coreProcessStarted) {
			DWORD error = GetLastError();
			OutputDebugStringA("failed to create core connection\n");
			return -2;
		}

		//remove if handles to process and primary thread are needed
		CloseHandle(procInfo.hProcess);
		CloseHandle(procInfo.hThread);

		///////////////////////////
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
WindowCallback(HWND window,
UINT msg,
WPARAM wParam,
LPARAM lParam) {
	LRESULT result = 0;
	switch (msg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(window, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(window, &ps);
		} break;

		default:
		{
			result = DefWindowProc(window, msg, wParam, lParam);
			//            OutputDebugStringA("default\n");
		} break;
		
		return result;
	}
	return DefWindowProc(window, msg, wParam, lParam);
}
