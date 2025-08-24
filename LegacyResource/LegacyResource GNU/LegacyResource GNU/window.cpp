#include <windows.h>	
HWND startupwin(HINSTANCE hInstance,WNDPROC WindowProc) {
	// Register the window class.
	const char* CLASS_NAME = "Sample Window Class";

	WNDCLASSA wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)GetStockObject(COLOR_BTNFACE + 1);
	RegisterClassA(&wc); // Use RegisterClassA for ANSI version
	HWND hwnd = CreateWindowExA(0, CLASS_NAME, "Legacy Resource", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, GetModuleHandle(NULL), NULL);
	ShowWindow(hwnd, SW_SHOW);
	return hwnd;
}


bool CALLBACK SetFont(HWND child, LPARAM font) {
	SendMessage(child, WM_SETFONT, font, true);
	return true;
}
