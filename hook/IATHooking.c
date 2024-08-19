#include <windows.h>
#include <stdio.h>

typedef int (WINAPI *MessageBoxA_t)(HWND, LPCSTR, LPCSTR, UINT);
MessageBoxA_t originalMessageBoxA;

int WINAPI HookedMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    // Modify the text of the MessageBox
    lpText = "Hooked!";
    
    // Call the original MessageBoxA
    return originalMessageBoxA(hWnd, lpText, lpCaption, uType);
}

void HookIAT(LPCTSTR moduleName, LPCSTR funcName, LPVOID hookFunc) {
    HMODULE hModule = GetModuleHandle(NULL); // Current process
    if (hModule == NULL) return;
    
    // Locate the IAT
    PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((BYTE *)hModule + pDOSHeader->e_lfanew);
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE *)hModule +
                                           pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    
    // Traverse the import descriptor to find the target module
    while (pImportDesc->Name) {
        LPCSTR pModuleName = (LPCSTR)((BYTE *)hModule + pImportDesc->Name);
        if (_stricmp(pModuleName, moduleName) == 0) {
            // Traverse the import address table
            PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((BYTE *)hModule + pImportDesc->FirstThunk);
            while (pThunk->u1.Function) {
                PROC *pFuncAddr = (PROC *)&pThunk->u1.Function;
                if (*pFuncAddr == (PROC)GetProcAddress(GetModuleHandle(moduleName), funcName)) {
                    DWORD oldProtect;
                    VirtualProtect(pFuncAddr, sizeof(LPVOID), PAGE_EXECUTE_READWRITE, &oldProtect);
                    originalMessageBoxA = (MessageBoxA_t)*pFuncAddr; // Save the original function pointer
                    *pFuncAddr = (PROC)hookFunc; // Replace with the hook function
                    VirtualProtect(pFuncAddr, sizeof(LPVOID), oldProtect, &oldProtect);
                    return;
                }
                pThunk++;
            }
        }
        pImportDesc++;
    }
}

int main() {
    // Hook the MessageBoxA function in the IAT
    HookIAT("user32.dll", "MessageBoxA", HookedMessageBoxA);

    // Test the hook
    MessageBoxA(NULL, "Original Message", "Test", MB_OK);

    return 0;
}
