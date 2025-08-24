#include <windows.h>

// Forward declaration of the function

HWND gaouserhwnd = GetForegroundWindow(); // Assuming gaouserhwnd is the foreground window


bool GetTransGenderEx(HWND hwnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    float transpercent = 0;
        transpercent = GetTransGenderEx(gaouserhwnd);
}

// Definition of the function
bool LGetTransGenderEx(HWND hwnd) {
    // Function implementation here
}