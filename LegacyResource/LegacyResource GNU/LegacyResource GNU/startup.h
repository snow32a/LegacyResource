#ifndef STARTUP_H
#define STARTUP_H

#include <windows.h>

// Function to create a window with a button and block until the window is closed
bool CreateStartupWindow(HINSTANCE hInstance);
bool CALLBACK SetFont(HWND child, LPARAM font);
#endif // STARTUP_H