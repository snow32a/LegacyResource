#include "dialogedit.h"
#include <commctrl.h>
#include "crtrep.h"
#include <windows.h>
#include <string>
#include <sstream>
static HWND hwndDialogPreview = NULL;
static HWND hwndTabControl = NULL;
static HWND hwndBinaryView = NULL;
LRESULT CALLBACK TabControlProc(HWND hTab, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
// Clean up dialog preview resources
void CleanupDialogPreview() {
    if (hwndDialogPreview) {
        DestroyWindow(hwndDialogPreview);
        hwndDialogPreview = NULL;
    }
    if (hwndBinaryView) {
        DestroyWindow(hwndBinaryView);
        hwndBinaryView = NULL;
    }
    if (hwndTabControl) {
        DestroyWindow(hwndTabControl);
        hwndTabControl = NULL;
    }
}
INT_PTR CALLBACK DialogPreviewProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        // Initialization code if needed
        return TRUE;

    case WM_COMMAND:
        // Handle button clicks or other commands
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, 0);
        return TRUE;
    }
    return FALSE; // Let system handle other messages
}

int DialogEditor(HMODULE dll, LPCSTR res, HWND hwndMain) {
    CleanupDialogPreview(); // Clean up any existing preview

    hwndDialogPreview = CreateDialogA(dll, res, hwndMain, NULL);
    if (hwndDialogPreview) {
        ShowWindow(hwndDialogPreview, SW_SHOW);
        return 1;
    }
    return 0;
}
void DisplayDialogResource(HWND hwnd, HMODULE hModule, LPCSTR lpszName, WORD wLang) {
    CleanupDialogPreview(); // Clean up previous dialog preview if any

    RECT rc;
    GetClientRect(hwnd, &rc);
    int leftMargin = 260;
    int rightMargin = 10;
    int topMargin = 50;
    int bottomMargin = 10;

    int width = rc.right - leftMargin - rightMargin;
    int height = rc.bottom - topMargin - bottomMargin;

    // Initialize common controls
    INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX), ICC_TAB_CLASSES };
    if (!InitCommonControlsEx(&icc)) {
        MessageBoxA(NULL, "Failed to initialize common controls", "Error", MB_OK);
        return;
    }
    // Fix for the MessageBoxA call with too few arguments.  
    // GetWindowTextA requires a buffer and its size to retrieve the window text.  
    // Correcting the function call to properly retrieve and display the window text.  
    /*
    char windowText[256]; // Buffer to store the window text.  
    GetWindowTextA(hwnd, windowText, sizeof(windowText)); // Retrieve the window text.  
    MessageBoxA(hwnd, windowText, "Debug", MB_OK); // Display the retrieved text in a message box.
    */
    // Create tab control
    hwndTabControl = CreateWindowExA(
        0,                          // No extended styles
        WC_TABCONTROLA,             // Use the ANSI version (consistent with your code)
        NULL,                       // No window text
        WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS,  // Basic styles
        leftMargin, topMargin,      // Position
        width, height,              // Size
        hwnd,                       // Parent window
        (HMENU)1000,                // Child ID
        GetModuleHandleA(NULL),     // Instance handle
        NULL                        // No additional parameters
    );

    if (!hwndTabControl) {
        DWORD err = GetLastError();
        char msg[256];
        wsprintfA(msg, "Failed to create tab control. Error code: %lu", err);
        MessageBoxA(NULL, msg, "Error", MB_OK);
       
    }      
    // Add tabs
    TCITEMA tie = { 0 };
    tie.mask = TCIF_TEXT;

    char tab1Text[] = "Binary View";
    tie.pszText = tab1Text;
    TabCtrl_InsertItem(hwndTabControl, 0, &tie);

    char tab2Text[] = "Dialog Preview";
    tie.pszText = tab2Text;
    TabCtrl_InsertItem(hwndTabControl, 1, &tie);

    // Create binary view edit control (child of tab control)

    hwndBinaryView = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        "EDIT",
        NULL,
        WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        5, 35,                         // Position relative to tab control
        width - 10, height - 20,       // Size
        hwndTabControl,                // Parent is the tab control
        NULL,
        GetModuleHandleA(NULL),
        NULL
    );

    // Load binary data
    HRSRC hRes = FindResourceExA(hModule, RT_DIALOG, lpszName, wLang);
    if (hRes) {
        HGLOBAL hLoaded = LoadResource(hModule, hRes);
        if (hLoaded) {
            LPVOID pData = LockResource(hLoaded);
            DWORD size = SizeofResource(hModule, hRes);

            char hexDump[4096] = { 0 };
            char temp[16];
            BYTE* bytes = (BYTE*)pData;

            for (DWORD i = 0; i < min(size, 256); i++) {
                if (i % 16 == 0) {
                    if (i > 0) {
                        StringConcatA(hexDump, "\r\n");
                    }
                    StringConcatA(hexDump, "0x");
                    char addrStr[8];
                    wsprintfA(addrStr, "%04X: ", i);
                    StringConcatA(hexDump, addrStr);
                }
                wsprintfA(temp, "%02X ", bytes[i]);
                StringConcatA(hexDump, temp);
            }

            SetWindowTextA(hwndBinaryView, hexDump);
        }
    }



    // Set up tab control handling
    SetWindowSubclass(hwndTabControl, TabControlProc, 0, 0);
}

LRESULT CALLBACK TabControlProc(HWND hTab, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch (uMsg) {
    case WM_NOTIFY: {
        NMHDR* nmh = (NMHDR*)lParam;
        if (nmh->code == TCN_SELCHANGE) {
            int sel = TabCtrl_GetCurSel(hTab);

            // Show/hide the appropriate window
            ShowWindow(hwndBinaryView, sel == 0 ? SW_SHOW : SW_HIDE);
            if (hwndDialogPreview) {
                ShowWindow(hwndDialogPreview, sel == 1 ? SW_SHOW : SW_HIDE);
            }
        }
        break;
    }
    case WM_DESTROY:
        RemoveWindowSubclass(hTab, TabControlProc, 0);
        break;
    }
    return DefSubclassProc(hTab, uMsg, wParam, lParam);
}