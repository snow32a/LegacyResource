#ifndef DIALOGEDIT_H
#define DIALOGEDIT_H

#include <windows.h>

int DialogEditor(HMODULE dll, LPCSTR res, HWND hwndMain);
void DisplayDialogResource(HWND hwnd, HMODULE hModule, LPCSTR lpszName, WORD wLang);
void CleanupDialogPreview();

#endif