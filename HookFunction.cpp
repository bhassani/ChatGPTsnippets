#include <windows.h>

typedef int (WINAPI *MESSAGEBOX)(HWND, LPCWSTR, LPCWSTR, UINT);

int WINAPI MyMessageBox(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
    // Your custom code here before or after calling the original MessageBox
    // ...

    // Call the original MessageBox function
    MESSAGEBOX pOrigMessageBox = (MESSAGEBOX)GetProcAddress(GetModuleHandle(L"user32.dll"), "MessageBoxW");
    return pOrigMessageBox(hWnd, lpText, lpCaption, uType);
}

int main()
{
    // Get a handle to the user32.dll module
    HMODULE hUser32 = GetModuleHandle(L"user32.dll");

    // Get the address of the original MessageBox function
    MESSAGEBOX pOrigMessageBox = (MESSAGEBOX)GetProcAddress(hUser32, "MessageBoxW");

    // Replace MessageBox with your hook function
    DWORD dwOldProtect;
    VirtualProtect(pOrigMessageBox, sizeof(MESSAGEBOX), PAGE_EXECUTE_READWRITE, &dwOldProtect);
    *(DWORD_PTR*)pOrigMessageBox = (DWORD_PTR)MyMessageBox;
    VirtualProtect(pOrigMessageBox, sizeof(MESSAGEBOX), dwOldProtect, &dwOldProtect);

    // Now, when MessageBox is called, your hook function (MyMessageBox) will be invoked.
    MessageBox(NULL, L"Hello, I'm hooked!", L"Hooked MessageBox", MB_OK);

    // Clean up and restore the original MessageBox function
    VirtualProtect(pOrigMessageBox, sizeof(MESSAGEBOX), PAGE_EXECUTE_READWRITE, &dwOldProtect);
    *(DWORD_PTR*)pOrigMessageBox = (DWORD_PTR)pOrigMessageBox;
    VirtualProtect(pOrigMessageBox, sizeof(MESSAGEBOX), dwOldProtect, &dwOldProtect);

    return 0;
}
