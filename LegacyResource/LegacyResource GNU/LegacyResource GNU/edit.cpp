#include <windows.h>
#include <commctrl.h>
#include "window.h"
#include "edit.h"

#define UNICODE 0
#define _UNICODE 0
#include "crtrep.h"
#include "dialogedit.h"
// Forward declarations
HTREEITEM AddItemToTree(HWND hwndTV, char* text, HTREEITEM hParent, LPARAM lParam);
void EnumAllResources(HMODULE hModule);
BOOL CALLBACK EnumResTypesProc(HMODULE hModule, LPSTR lpszType, LONG_PTR lParam);
int CALLBACK EnumResNamesProc(HMODULE hModule, LPCSTR lpszType, LPSTR lpszName, LONG_PTR lParam);
BOOL CALLBACK EnumResLangProc(HMODULE hModule, LPCSTR lpszType, LPCSTR lpszName, WORD wLangId, LONG_PTR lParam);
void DisplayResource(HWND hwnd, HMODULE hModule, LPCSTR lpszType, LPCSTR lpszName, WORD wLang);
void CleanupTreeViewItems(HWND hwndTV);
BOOL OpenResourceFile(HWND hwnd);

// Global variables
HWND hwndTV;            // Tree view window
HWND hwndPreview;       // Preview window
HMODULE g_hCurrentModule = NULL; // Currently loaded module


// Resource item information structure
typedef struct {
    LONG_PTR resType;    // Original resource type value
    LONG_PTR resName;    // Original resource name value
    WORD langId;         // Language ID (for language items)
    int itemType;        // 0=Type, 1=Name, 2=Lang, 3=Property
} RESITEMINFO;

// Create a resource ID structure 
LPARAM CreateResItemInfo(LONG_PTR resType, LONG_PTR resName, WORD langId, int itemType) {
    RESITEMINFO* info = (RESITEMINFO*)MemAlloc(sizeof(RESITEMINFO));
    if (info) {
        info->resType = resType;
        info->resName = resName;
        info->langId = langId;
        info->itemType = itemType;
    }
    return (LPARAM)info;
}

BOOL CALLBACK EnumResLangProc(HMODULE hModule, LPCSTR lpszType, LPCSTR lpszName,
    WORD wLangId, LONG_PTR lParam) {
    HWND hwndTV = (HWND)((LPARAM)lParam >> 32);
    HTREEITEM hParent = (HTREEITEM)((LPARAM)lParam & 0xFFFFFFFF);

    char langStr[64];
    StringCopyA(langStr, "Language: ");
    IntToStringA(wLangId, langStr + lstrlenA(langStr));

    HTREEITEM hLang = AddItemToTree(hwndTV, langStr, hParent,
        CreateResItemInfo((LONG_PTR)lpszType, (LONG_PTR)lpszName, wLangId, 2));

    HRSRC hResInfo = FindResourceExA(hModule, lpszType, lpszName, wLangId);
    if (hResInfo != NULL) {
        DWORD size = SizeofResource(hModule, hResInfo);

        char sizeStr[64];
        StringCopyA(sizeStr, "Size: ");
        IntToStringA(size, sizeStr + lstrlenA(sizeStr));
        StringConcatA(sizeStr, " bytes");

        AddItemToTree(hwndTV, sizeStr, hLang,
            CreateResItemInfo((LONG_PTR)lpszType, (LONG_PTR)lpszName, wLangId, 3));
    }

    return TRUE;
}

int CALLBACK EnumResNamesProc(HMODULE hModule, LPCSTR lpszType, LPSTR lpszName, LONG_PTR lParam) {
    static HWND hwndTV = NULL;
    if (hwndTV == NULL) {
        HWND hwnd = FindWindowA("LegacyResourceEdit", NULL);
        if (hwnd != NULL) {
            hwndTV = FindWindowExA(hwnd, NULL, WC_TREEVIEWA, NULL);
        }
    }

    char typeStr[64];
    typeStr[0] = '\0';

    if (IS_INTRESOURCE(lpszType)) {
        switch ((INT_PTR)lpszType) {
        case 1:       StringCopyA(typeStr, "Cursor"); break;
        case 2:       StringCopyA(typeStr, "Bitmap"); break;
        case 3:       StringCopyA(typeStr, "Icon"); break;
        case 4:       StringCopyA(typeStr, "Menu"); break;
        case 5:       StringCopyA(typeStr, "Dialog"); break;
        case 6:       StringCopyA(typeStr, "String Table"); break;
        case 7:      StringCopyA(typeStr, "Font Directory"); break;
        case 8:       StringCopyA(typeStr, "Font"); break;
        case 9:  StringCopyA(typeStr, "Accelerator"); break;
        case 10:      StringCopyA(typeStr, "RC Data"); break;
        case 11: StringCopyA(typeStr, "Message Table"); break;
        case 12:      StringCopyA(typeStr, "Version"); break;
        case 13:   StringCopyA(typeStr, "Dialog Include"); break;
        case 14:     StringCopyA(typeStr, "Plug & Play"); break;
        case 15:      StringCopyA(typeStr, "VXD"); break;
        case 16:    StringCopyA(typeStr, "Animated Cursor"); break;
        case 17:      StringCopyA(typeStr, "Animated Icon"); break;
        case 18:       StringCopyA(typeStr, "HTML"); break;
        case 19:     StringCopyA(typeStr, "Manifest"); break;
        default:
            StringCopyA(typeStr, "Resource #");
            IntToStringA((INT_PTR)lpszType, typeStr + lstrlenA(typeStr));
            break;
        }
    }
    else {
        StringCopyA(typeStr, lpszType);
    }

    HTREEITEM hRoot = NULL;
    HTREEITEM hItem = TreeView_GetRoot(hwndTV);
    char itemText[256];
    TVITEMA tvi;

    while (hItem != NULL) {
        tvi.mask = TVIF_TEXT | TVIF_PARAM;
        tvi.pszText = itemText;
        tvi.cchTextMax = sizeof(itemText);
        tvi.hItem = hItem;
        SendMessageA(hwndTV, TVM_GETITEMA, 0, (LPARAM)&tvi);

        RESITEMINFO* info = (RESITEMINFO*)tvi.lParam;
        if (info && info->itemType == 0) {
            if ((IS_INTRESOURCE(lpszType) && IS_INTRESOURCE((LPCSTR)info->resType) &&
                (INT_PTR)lpszType == (INT_PTR)info->resType) ||
                (!IS_INTRESOURCE(lpszType) && !IS_INTRESOURCE((LPCSTR)info->resType) &&
                    lstrcmpA(lpszType, (LPCSTR)info->resType) == 0)) {
                hRoot = hItem;
                break;
            }
        }

        hItem = TreeView_GetNextSibling(hwndTV, hItem);
    }

    if (hRoot == NULL) {
        hRoot = AddItemToTree(hwndTV, typeStr, TVI_ROOT,
            CreateResItemInfo((LONG_PTR)lpszType, 0, 0, 0));
    }

    char nameStr[256];
    nameStr[0] = '\0';

    if (IS_INTRESOURCE(lpszName)) {
        StringCopyA(nameStr, "ID: ");
        IntToStringA((INT_PTR)lpszName, nameStr + lstrlenA(nameStr));
    }
    else {
        StringCopyA(nameStr, lpszName);
    }

    HTREEITEM hName = AddItemToTree(hwndTV, nameStr, hRoot,
        CreateResItemInfo((LONG_PTR)lpszType, (LONG_PTR)lpszName, 0, 1));

    EnumResourceLanguagesA(hModule, lpszType, lpszName, EnumResLangProc,
        (LONG_PTR)((LPARAM)hName | ((LPARAM)hwndTV << 32)));

    return TRUE;
}

int CALLBACK EnumResTypesProc(HMODULE hModule, LPSTR lpszType, LONG_PTR lParam) {
    EnumResourceNamesA(hModule, lpszType, EnumResNamesProc, (LONG_PTR)lpszType);
    return TRUE;
}

void EnumAllResources(HMODULE hModule) {
    if (hModule == NULL) {
        MessageBoxA(NULL, "Failed to load the specified module!", "Error", MB_ICONERROR);
        return;
    }

    TreeView_DeleteAllItems(hwndTV);
    EnumResourceTypesA(hModule, EnumResTypesProc, 0);
}

BOOL OpenResourceFile(HWND hwnd) {
    char fileName[MAX_PATH] = "";

    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Executable Files (*.exe;*.dll)\0*.exe;*.dll\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        if (g_hCurrentModule) {
            FreeLibrary(g_hCurrentModule);
        }
        g_hCurrentModule = LoadLibraryExA(fileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (g_hCurrentModule) {
            EnumAllResources(g_hCurrentModule);
            return TRUE;
        }
    }
    return FALSE;
}

void DisplayResource(HWND hwnd, HMODULE hModule, LPCSTR lpszType, LPCSTR lpszName, WORD wLang) {
    if (hwndPreview) {
        DestroyWindow(hwndPreview);
        hwndPreview = NULL;
    }

    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.left = 260;
    rc.right -= 10;
    rc.top = 50;
    rc.bottom -= 10;

    if (IS_INTRESOURCE(lpszType)) {
        switch ((INT_PTR)lpszType) {
        case 2: // BITMAP
        case 3: // ICON
        {
            hwndPreview = CreateWindowExA(0, "STATIC", NULL,
                WS_VISIBLE | WS_CHILD | SS_BITMAP,
                rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                hwnd, NULL, GetModuleHandleA(NULL), NULL);

            if ((INT_PTR)lpszType == 2) { // BITMAP
                HBITMAP hBmp = LoadBitmapA(hModule, lpszName);
                if (hBmp) {
                    SendMessageA(hwndPreview, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
                }
            }
            else if ((INT_PTR)lpszType == 3) { // ICON
                HICON hIcon = LoadIconA(hModule, lpszName);
                if (hIcon) {
                    SendMessageA(hwndPreview, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
                }
            }
            break;
        }
        case 5: // DIALOG
            DisplayDialogResource(hwnd, hModule, lpszName, wLang);
            break;

        case 6: // STRING TABLE
        {
            hwndPreview = CreateWindowExA(0, "EDIT", NULL,
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                hwnd, NULL, GetModuleHandleA(NULL), NULL);

            char stringContent[4096] = { 0 };
            HRSRC hRes = FindResourceExA(hModule, lpszType, lpszName, wLang);
            if (hRes) {
                HGLOBAL hLoaded = LoadResource(hModule, hRes);
                if (hLoaded) {
                    LPVOID pData = LockResource(hLoaded);
                    StringCopyA(stringContent, (LPCSTR)pData);
                    SetWindowTextA(hwndPreview, stringContent);
                }
            }
            break;
        }
        default:
            hwndPreview = CreateWindowExA(0, "EDIT", "Preview not available for this resource type",
                WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_READONLY,
                rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                hwnd, NULL, GetModuleHandleA(NULL), NULL);
            break;
        }
    }
    else {
        hwndPreview = CreateWindowExA(0, "EDIT", "Preview not available for this resource type",
            WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_READONLY,
            rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
            hwnd, NULL, GetModuleHandleA(NULL), NULL);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HFONT hFont = NULL;

    switch (uMsg) {
    case WM_DESTROY:
        if (g_hCurrentModule) {
            FreeLibrary(g_hCurrentModule);
        }
        CleanupTreeViewItems(hwndTV);
        PostQuitMessage(0);
        return 0;

    case WM_CREATE:
    {
        HMENU hMenu = CreateMenu();
        HMENU hFileMenu = CreatePopupMenu();
        AppendMenuA(hFileMenu, MF_STRING, 1001, "&Open");
        AppendMenuA(hFileMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(hFileMenu, MF_STRING, 1002, "&Exit");
        AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "&File");
        SetMenu(hwnd, hMenu);

        hFont = CreateFontA(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Trebuchet MS");

        hwndTV = CreateWindowExA(0, WC_TREEVIEWA, "TreeView",
            WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
            10, 50, 240, 400, hwnd, NULL, GetModuleHandleA(NULL), NULL);

        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1001: // Open file
            OpenResourceFile(hwnd);
            return 0;
        case 1002: // Exit
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_NOTIFY:
    {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->hwndFrom == hwndTV) {
            if (pnmh->code == TVN_SELCHANGED) {
                LPNMTREEVIEW pnmv = (LPNMTREEVIEW)lParam;
                HTREEITEM hSelectedItem = pnmv->itemNew.hItem;

                char itemText[256] = { 0 };
                TVITEMA tvi = { 0 };
                tvi.mask = TVIF_TEXT | TVIF_PARAM;
                tvi.hItem = hSelectedItem;
                tvi.pszText = itemText;
                tvi.cchTextMax = sizeof(itemText);

                if (SendMessageA(hwndTV, TVM_GETITEMA, 0, (LPARAM)&tvi)) {
                    RESITEMINFO* info = (RESITEMINFO*)tvi.lParam;
                    if (info && (info->itemType == 1 || info->itemType == 2)) {
                        DisplayResource(hwnd, g_hCurrentModule,
                            (LPCSTR)info->resType,
                            (LPCSTR)info->resName,
                            info->langId);
                    }
                }
            }
        }
        break;
    }

    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        if (hwndTV) {
            SetWindowPos(hwndTV, NULL, 10, 50, 240, height - 60, SWP_NOZORDER);
        }

        if (hwndPreview) {
            SetWindowPos(hwndPreview, NULL, 260, 50, width - 270, height - 60, SWP_NOZORDER);
        }
        break;
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
            RECT rc = { 20, 20, 500, 200 };
            DrawTextA(hdc, text, -1, &rc, DT_SINGLELINE | DT_LEFT | DT_TOP);
            SelectObject(hdc, hOldFont);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

HTREEITEM AddItemToTree(HWND hwndTV, char* text, HTREEITEM hParent, LPARAM lParam) {
    TVITEMA tvi;
    TVINSERTSTRUCTA tvins;

    tvi.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.pszText = (LPSTR)text;
    tvi.lParam = lParam;
    tvi.iImage = 0;
    tvi.iSelectedImage = 1;

    tvins.item = tvi;
    tvins.hInsertAfter = TVI_LAST;
    tvins.hParent = hParent;

    return (HTREEITEM)SendMessageA(hwndTV, TVM_INSERTITEMA, 0, (LPARAM)(LPTVINSERTSTRUCTA)&tvins);
}

void CleanupTreeViewItems(HWND hwndTV) {
    HTREEITEM hItem = TreeView_GetRoot(hwndTV);

    while (hItem) {
        TVITEMA tvi;
        tvi.mask = TVIF_PARAM | TVIF_HANDLE;
        tvi.hItem = hItem;

        if (SendMessageA(hwndTV, TVM_GETITEMA, 0, (LPARAM)&tvi) && tvi.lParam) {
            MemFree((void*)tvi.lParam);
        }

        hItem = TreeView_GetNextSibling(hwndTV, hItem);
    }
}

bool CreateEditorWindow(HINSTANCE hInstance) {
    const char* CLASS_NAME = "LegacyResourceEdit";

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClassA(&wc)) {
        return false;
    }

    HWND hwnd = CreateWindowExA(0, CLASS_NAME, "LegacyResource",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return false;
    }



    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg = { 0 };
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return true;
}