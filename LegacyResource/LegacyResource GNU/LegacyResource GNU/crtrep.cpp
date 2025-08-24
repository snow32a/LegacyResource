
#include "crtrep.h"
#include <windows.h>

int StringLength(const char* str) {
    int len = 0;
    while (*str++) len++;
    return len;
}

void StringCopy(char* dest, const char* src) {
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

// Memory functions without CRT
void* MemAlloc(SIZE_T size) {
    return HeapAlloc(GetProcessHeap(), 0, size);
}

void MemFree(void* ptr) {
    HeapFree(GetProcessHeap(), 0, ptr);
}

// Helper function to copy strings without CRT
void StringCopyA(LPSTR dest, LPCSTR src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

// Helper function to convert int to string without CRT
void IntToStringA(int value, LPSTR buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    int isNegative = 0;
    if (value < 0) {
        isNegative = 1;
        value = -value;
    }

    char temp[16];
    int i = 0;

    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    int bufPos = 0;
    if (isNegative) {
        buffer[bufPos++] = '-';
    }

    while (i > 0) {
        buffer[bufPos++] = temp[--i];
    }

    buffer[bufPos] = '\0';
}

// Helper function to combine strings without CRT
void StringConcatA(LPSTR dest, LPCSTR src) {
    while (*dest) dest++;
    while (*src) *dest++ = *src++;
    *dest = '\0';
}
