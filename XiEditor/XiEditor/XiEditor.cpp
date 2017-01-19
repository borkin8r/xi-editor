// Xi-Editor.cpp : Defines the entry point for the application.
//

#include "XiEditor.h"

typedef char* (*RPCCallback)();
typedef unsigned long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef long l64;
typedef int i32;
typedef short s16;
typedef char c8;
LRESULT CALLBACK WindowCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
u64 SendRPCAsync(void* message);
void CreateNewTab(u64* threadId);

/////////////////////
// Globals
static PROCESS_INFORMATION coreProcInfo;
static HANDLE coreInputWriteEnd = NULL;
static HANDLE coreInputReadEnd = NULL;
static HANDLE coreOutputWriteEnd = NULL;
static HANDLE coreOutputReadEnd = NULL;
static STARTUPINFO startInfo;

int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prevInstance,
		LPTSTR    commandLine, int       cmdShow)
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
			0,															// Optional window styles.
			windowClass.lpszClassName,									 // Window class
			"Xi-Editor text",											// Window text
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,							// Window style
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Size and position
			NULL,														// Parent window    
			NULL,														// Menu
			instance,													// Instance handle
			NULL														// Additional application data
			);

		if (window == NULL)
		{
			return 0;
		}

		ShowWindow(window, cmdShow);

		///////////////////////////////
		//Setup connection to Xi-Core

		//TODO: zero memory?
		coreProcInfo = { 0 };
		startInfo = { 0 };
		coreInputWriteEnd = NULL; //lose inheritance
		coreInputReadEnd = NULL; //thread lock
		coreOutputWriteEnd = NULL; //thread lock
		coreOutputReadEnd = NULL; //lose inheritance
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

		unsigned long flags = 0;

		GetHandleInformation(coreInputWriteEnd, &flags);
		OutputDebugStringA("debug\n");

		startInfo.cb = sizeof(STARTUPINFO);
		startInfo.hStdError = coreOutputWriteEnd;
		startInfo.hStdOutput = coreOutputWriteEnd;
		startInfo.hStdInput = coreInputReadEnd;
		startInfo.dwFlags = STARTF_USESTDHANDLES;

		///////////////////////////
		// create core connection

		BOOL coreProcessStarted = FALSE;
		//TCHAR NPath[MAX_PATH];
		//GetCurrentDirectory(MAX_PATH, NPath);

		//DWORD dwAttrib = GetFileAttributes("..\\..\\rust\\target\\debug\\xi-core.exe");

		coreProcessStarted = CreateProcess(
			"..\\..\\rust\\target\\debug\\xi-core.exe",		   // path to executable "xi-core",
			NULL,												// command line arguements
			NULL,												// process security attributes 
			NULL,												// primary thread security attributes 
			TRUE,												// handles are inherited 
			CREATE_NO_WINDOW,									// creation flags 
			NULL,												// use parent's environment 
			NULL,												// use parent's current directory 
			&startInfo,											// STARTUPINFO pointer 
			&coreProcInfo);										// receives PROCESS_INFORMATION
		if (!coreProcessStarted) {
			DWORD error = GetLastError();
			OutputDebugStringA("failed to create core connection\n");
			return -2;
		}

		///////////////////
		//create send message array
		UINT32 maxSendMessages = 100;
		UINT32 maxMessageSize = 1000;
		char** sendMessageQueue = (char**) malloc(sizeof(char*) * maxSendMessages); //TODO: free?
		for (int i = 0; i < maxSendMessages; i++) {
			sendMessageQueue[i] = (char*)calloc(maxMessageSize, sizeof(char));
		}


		u64 sendThreadID = 0;
		CreateNewTab(&sendThreadID);

		///////////////////////////
		// Run the message loop.

		MSG message = {};
		while (GetMessage(&message, NULL, 0, 0))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
			Sleep(1000);
		}
		//CloseHandle(coreOutputReadEnd);
		ExitProcess(0);
	}
	return 0;
}


void
CreateNewTab(u64* threadId) {
	////////////////
	//fill buffer with json message "new_tab", ""
	char newTabMessage[] = "{\"id\":0,\"method\":\"new_tab\",\"params\":[]}\n"; //TODO: create on heap!
	
	CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		SendRPCAsync,       // thread function name
		newTabMessage,          // argument to thread function 
		0,                      // use default creation flags 
		threadId);   // returns the thread identifier
}

u64 
SendRPCAsync(void* data) //TODO: pass params to uiMessage, call CreateThread to do the work
{
	////////////////////////////////////////
	//refactor out to thread messaging code
	char* uiMessage = (char*) data; //NOTE: Messages to core must be appended with a new line
	u64 uiMessageSize = strlen(uiMessage); //NOTE: must be null terminated string
	u64 uiBytesWritten = 0;

	if (WriteFile(coreInputWriteEnd, uiMessage, strlen(uiMessage), &uiBytesWritten, NULL) == 0) {
		u64 e = GetLastError(); //TODO: handle error
		
	}
	//TODO: free message from heap
}

LRESULT CALLBACK
WindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT result = 0;
	switch (message)
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
			result = DefWindowProc(window, message, wParam, lParam);
			//            OutputDebugStringA("default\n");
		} break;
		
		return result;
	}
	return DefWindowProc(window, message, wParam, lParam);
}
