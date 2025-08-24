#include "startup.h"
#include <windows.h>
#include <string>
#include "window.h"
// Global variables
bool leave = false;
bool leaveval = false;
HWND hwndok;
HWND hwndcancel;


LRESULT CALLBACK WindowProcStart(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HFONT hFont = NULL;
    static HFONT hFont2 = NULL;
    switch (uMsg) {
    case WM_COMMAND:
        if (LOWORD(wParam) == 1 && (HWND)lParam == hwndok) {
            leave = true;
            leaveval = true;
            DestroyWindow(hwnd);
        }
        else if (LOWORD(wParam) == 2 && (HWND)lParam == hwndcancel) {
            leave = true;
            leaveval = false;
            DestroyWindow(hwnd);
        }
        break;

    case WM_DESTROY:
        leave = true;
        return 0;
    case WM_CREATE:
    {
        hFont = CreateFontA(
            48,
            0,
            0,
            0,
            FW_BOLD,
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
        hFont2 = CreateFont(
            24,
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

            HFONT hOldFont2 = (HFONT)SelectObject(hdc, hFont2);

            const char* text2 = "To rock at modifying PE's!";
            SetTextColor(hdc, RGB(0, 0, 0));

            SetBkMode(hdc, TRANSPARENT);

            RECT rc2 = { 50, 100, 500, 200 };

            DrawTextA(hdc, text2, -1, &rc2, DT_SINGLELINE | DT_LEFT | DT_TOP);
            SelectObject(hdc, hOldFont);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

bool CreateStartupWindow(HINSTANCE hInstance) {
    // Reset global variables
    leave = false;
    leaveval = false;

    const char* CLASS_NAME = "StartupWindowClass";

    WNDCLASSA wc = { };
    wc.lpfnWndProc = WindowProcStart;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Failed to register window class!", "Error", MB_ICONERROR);
        return false;
    }

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "LegacyResource",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX ^ WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create window!", "Error", MB_ICONERROR);
        return false;
    }

    hwndok = CreateWindowExA(
        0,
        "BUTTON",
        "Load",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        400, 320,
        75, 25,
        hwnd,
        (HMENU)1,
        GetModuleHandle(NULL),
        NULL
    );

    if (hwndok == NULL) {
        MessageBoxA(NULL, "Failed to create button!", "Error", MB_ICONERROR);
        DestroyWindow(hwnd);
        return false;
    }

    hwndcancel = CreateWindowExA(
        0,
        "BUTTON",
        "Cancel",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        480, 320,
        75, 25,
        hwnd,
        (HMENU)2,
        GetModuleHandle(NULL),
        NULL
    );

    if (hwndcancel == NULL) {
        MessageBoxA(NULL, "Failed to create cancel button!", "Error", MB_ICONERROR);
        DestroyWindow(hwndok);
        DestroyWindow(hwnd);
        return false;
    }

    ShowWindow(hwnd, SW_SHOW);

    EnumChildWindows(hwnd, (WNDENUMPROC)SetFont, (LPARAM)GetStockObject(DEFAULT_GUI_FONT));

    MSG msg = { };
    while (!leave) {
        // Use PeekMessage instead of GetMessage to allow checking leave flag
        if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    // Make sure window is properly destroyed

    // Return the value set by button press
    return leaveval;
}