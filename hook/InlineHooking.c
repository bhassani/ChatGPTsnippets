#include <windows.h>
#include <stdio.h>

BYTE originalBytes[5];
DWORD targetFuncAddr;

// A sample replacement function for MessageBoxA
int WINAPI HookedMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    // Modify the text of the MessageBox
    lpText = "Hooked!";
    
    // Call the original function by restoring the original bytes
    DWORD oldProtect;
    VirtualProtect((LPVOID)targetFuncAddr, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((LPVOID)targetFuncAddr, originalBytes, 5);
    VirtualProtect((LPVOID)targetFuncAddr, 5, oldProtect, &oldProtect);
    
    int result = MessageBoxA(hWnd, lpText, lpCaption, uType);

    // Re-hook the function
    VirtualProtect((LPVOID)targetFuncAddr, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    BYTE jmp[5] = { 0xE9, 0, 0, 0, 0 };
    DWORD relativeAddr = ((DWORD)HookedMessageBoxA - targetFuncAddr - 5);
    memcpy(jmp + 1, &relativeAddr, sizeof(relativeAddr));
    memcpy((LPVOID)targetFuncAddr, jmp, sizeof(jmp));
    VirtualProtect((LPVOID)targetFuncAddr, 5, oldProtect, &oldProtect);
    
    return result;
}

void HookFunction(LPCTSTR moduleName, LPCSTR funcName, LPVOID hookFunc) {
    HMODULE hModule = GetModuleHandle(moduleName);
    if (hModule == NULL) return;
    
    targetFuncAddr = (DWORD)GetProcAddress(hModule, funcName);
    if (targetFuncAddr == 0) return;
    
    // Backup original bytes
    memcpy(originalBytes, (LPVOID)targetFuncAddr, 5);
    
    // Write JMP to hook function
    DWORD oldProtect;
    VirtualProtect((LPVOID)targetFuncAddr, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    BYTE jmp[5] = { 0xE9, 0, 0, 0, 0 };
    DWORD relativeAddr = ((DWORD)hookFunc - targetFuncAddr - 5);
    memcpy(jmp + 1, &relativeAddr, sizeof(relativeAddr));
    memcpy((LPVOID)targetFuncAddr, jmp, sizeof(jmp));
    VirtualProtect((LPVOID)targetFuncAddr, 5, oldProtect, &oldProtect);
}

int main() {
    // Hook the MessageBoxA function in user32.dll
    HookFunction("user32.dll", "MessageBoxA", HookedMessageBoxA);

    // Test the hook
    MessageBoxA(NULL, "Original Message", "Test", MB_OK);

    return 0;
}
