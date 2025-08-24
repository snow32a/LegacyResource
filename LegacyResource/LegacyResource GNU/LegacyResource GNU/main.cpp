#include <windows.h>
#include "window.h"
#include "startup.h"
#include "edit.h"
#include <CommCtrl.h>
// String functions without CRT
LRESULT CALLBACK MWProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HFONT hFont = NULL;
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CREATE:
    {
        hFont = CreateFont(
            48,
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            "Trebuchet MS"
        );

        if (hFont == NULL) {
            MessageBox(NULL, "Failed to create font!", "Error", MB_OK | MB_ICONERROR);
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_3DFACE + 1));

        if (hFont != NULL) {
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

            const char* text = "LegacyResource";

            SetTextColor(hdc, RGB(0, 0, 0));
            SetBkMode(hdc, TRANSPARENT);

            RECT rc = { 50, 50, 500, 200 };

            DrawTextA(hdc, text, -1, &rc, DT_SINGLELINE | DT_LEFT | DT_TOP);

            SelectObject(hdc, hOldFont);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}


// Custom entry point for no CRT builds
void WINAPI WinMainCRTStartup() {
    // Get the command line (CRT usually processes this, so we do it manually)
    LPSTR lpCmdLine = GetCommandLineA();

    // Extract nCmdShow (normally provided by the system)
    STARTUPINFOA si;
    GetStartupInfoA(&si);
    int nCmdShow = (si.dwFlags & STARTF_USESHOWWINDOW) ? si.wShowWindow : SW_SHOWDEFAULT;

    // Call user-defined WinMain
    int exitCode = WinMain(GetModuleHandle(NULL), NULL, lpCmdLine, nCmdShow);

    // Exit properly without CRT
    ExitProcess(exitCode);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_TAB_CLASSES };
    InitCommonControlsEx(&icc);

    if (CreateStartupWindow(hInstance) == FALSE) {
        return 1;
    }
    CreateEditorWindow(hInstance);
}