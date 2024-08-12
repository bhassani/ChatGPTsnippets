#include <windows.h>
#include <stdio.h>

// Pointer to the original CreateWindowEx function
typedef HWND (WINAPI *CreateWindowExA_t)(
    DWORD     dwExStyle,
    LPCSTR    lpClassName,
    LPCSTR    lpWindowName,
    DWORD     dwStyle,
    int       x,
    int       y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
);
CreateWindowExA_t OriginalCreateWindowExA = NULL;

// Our custom function that will be called instead of CreateWindowExA
HWND WINAPI HookedCreateWindowExA(
    DWORD     dwExStyle,
    LPCSTR    lpClassName,
    LPCSTR    lpWindowName,
    DWORD     dwStyle,
    int       x,
    int       y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
) {
    MessageBox(NULL, "CreateWindowEx has been hooked!", "Hooked!", MB_OK);
    // Call the original function if needed or do something else
    return NULL;
}

// Function to hook the IAT entry for CreateWindowExA
void HookIAT() {
    // Get the base address of the module (the application)
    HMODULE hModule = GetModuleHandle(NULL);
    if (!hModule) {
        return;
    }

    // Get the DOS header and then the NT headers
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDosHeader->e_lfanew);

    // Get the import directory
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hModule +
        pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    // Iterate through the import descriptors to find the entry for user32.dll
    while (pImportDesc->Name) {
        const char* moduleName = (const char*)((BYTE*)hModule + pImportDesc->Name);
        if (_stricmp(moduleName, "user32.dll") == 0) {
            break;
        }
        pImportDesc++;
    }

    if (!pImportDesc->Name) {
        return;
    }

    // Get the first thunk which contains the addresses of the imported functions
    PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((BYTE*)hModule + pImportDesc->FirstThunk);

    // Iterate through the functions to find CreateWindowExA
    while (pThunk->u1.Function) {
        FARPROC* pFunc = (FARPROC*)&pThunk->u1.Function;
        if (*pFunc == (FARPROC)CreateWindowExA) {
            // Save the original address of CreateWindowExA
            OriginalCreateWindowExA = (CreateWindowExA_t)*pFunc;

            // Overwrite the IAT entry with the address of our custom function
            DWORD oldProtect;
            VirtualProtect(pFunc, sizeof(FARPROC), PAGE_EXECUTE_READWRITE, &oldProtect);
            *pFunc = (FARPROC)HookedCreateWindowExA;
            VirtualProtect(pFunc, sizeof(FARPROC), oldProtect, &oldProtect);
            break;
        }
        pThunk++;
    }
}

int main() {
    // Hook the IAT
    HookIAT();

    // Call CreateWindowExA to test the hook (this should now show the message box)
    CreateWindowExA(0, "STATIC", "Test Window", WS_OVERLAPPEDWINDOW, 0, 0, 300, 300, NULL, NULL, NULL, NULL);

    return 0;
}
