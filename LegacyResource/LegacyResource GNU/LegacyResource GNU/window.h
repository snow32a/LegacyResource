#ifndef STARTUPWIN_H
#define STARTUPWIN_H

#include <windows.h>

HWND startupwin(HINSTANCE hInstance, WNDPROC WindowProc);
bool CALLBACK SetFont(HWND child, LPARAM font);
#endif // STARTUPWIN_H#pragma once
