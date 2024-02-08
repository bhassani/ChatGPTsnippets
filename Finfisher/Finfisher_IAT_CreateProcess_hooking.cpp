/*
MyCreateProcess is the dummy function that will be called whenever CreateWindowEx is invoked after hooking. It simply prints a message and then calls the original CreateProcess function.

HookIAT is a function that performs the IAT hooking. It searches for the specified function (pszImportFuncName) within the Import Address Table of the specified module (pszImportModName) and replaces its address with the address of the hook function (pfnHookFunc).

In main(), HookIAT is called to hook CreateWindowExA from user32.dll to redirect it to MyCreateProcess. Then, a call to CreateWindowExA is made to demonstrate the redirection.
*/

#include <windows.h>
#include <stdio.h>

// Typedefine the original function pointer type
typedef BOOL(WINAPI *CREATEPROCESS)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);

// Define the prototype of our dummy function
BOOL WINAPI MyCreateProcess(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) {
    printf("Redirected CreateProcess called!\n");
    // Call the original CreateProcess function
    return ((CREATEPROCESS)GetProcAddress(GetModuleHandleA("kernel32.dll"), "CreateProcessA"))(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

// Function to perform IAT hooking
BOOL HookIAT(HMODULE hModule, LPCSTR pszImportModName, LPCSTR pszImportFuncName, PROC pfnHookFunc) {
    // Get the address of the Import Address Table
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return FALSE;
    }

    PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((BYTE *)pDosHeader + pDosHeader->e_lfanew);
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE *)pDosHeader + pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    // Iterate through the import descriptors
    while (pImportDesc->Name != NULL) {
        LPCSTR pszCurModName = (LPCSTR)((BYTE *)pDosHeader + pImportDesc->Name);

        // Check if this is the module we're interested in
        if (lstrcmpiA(pszCurModName, pszImportModName) == 0) {
            // Get the original function address
            PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((BYTE *)pDosHeader + pImportDesc->FirstThunk);
            while (pThunk->u1.Function != NULL) {
                PROC *ppfn = (PROC *)&pThunk->u1.Function;
                if (*ppfn == GetProcAddress(GetModuleHandleA(pszImportModName), pszImportFuncName)) {
                    DWORD dwOldProtect;
                    VirtualProtect(ppfn, sizeof(PROC), PAGE_READWRITE, &dwOldProtect);
                    *ppfn = pfnHookFunc;
                    VirtualProtect(ppfn, sizeof(PROC), dwOldProtect, &dwOldProtect);
                    return TRUE;
                }
                pThunk++;
            }
        }
        pImportDesc++;
    }
    return FALSE;
}

int main() {
    // Hook the CreateWindowEx function to redirect to CreateProcess
    if (!HookIAT(GetModuleHandle(NULL), "user32.dll", "CreateWindowExA", (PROC)MyCreateProcess)) {
        printf("Failed to hook CreateWindowEx.\n");
        return 1;
    }

    // Now, any call to CreateWindowEx will redirect to MyCreateProcess
    CreateWindowExA(0, "BUTTON", "Hello", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);

    return 0;
}

