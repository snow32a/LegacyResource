//just make like the header stuff for the code. not the functions, the header for crtrep.c

#include <windows.h>
// Function declarations  
int StringLength(const char* str);  
void StringCopy(char* dest, const char* src);  

// Memory functions without CRT  
void* MemAlloc(SIZE_T size);  
void MemFree(void* ptr);  

// Helper functions  
void StringCopyA(LPSTR dest, LPCSTR src);  
int min(int a, int b);  
void IntToStringA(int value, LPSTR buffer);  
void StringConcatA(LPSTR dest, LPCSTR src);

