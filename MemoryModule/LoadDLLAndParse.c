#include <stdio.h>
#include <windows.h>

// Define function pointer type for the functions we want to call from the DLL
typedef int (*AddFunc)(int, int);

// Helper function to get the base address of a loaded module by its name
DWORD GetModuleBaseAddress(const char* moduleName) {
    HMODULE hModule = GetModuleHandleA(moduleName);
    if (hModule == NULL) {
        return 0;
    }
    return (DWORD)hModule;
}

int main() {
    // Load the DLL into memory
    HMODULE hModule = LoadLibrary("example.dll");
    if (hModule == NULL) {
        printf("Failed to load DLL.\n");
        return 1;
    }

    // Get the address of the entry point function
    FARPROC entryPoint = GetProcAddress(hModule, "DllMain");
    if (entryPoint == NULL) {
        printf("Failed to get entry point address.\n");
        FreeLibrary(hModule);
        return 1;
    }

    // Call the entry point function
    int result = ((int(*)(HMODULE, DWORD, LPVOID))entryPoint)(hModule, DLL_PROCESS_ATTACH, NULL);
    if (result != TRUE) {
        printf("Failed to execute entry point function.\n");
        FreeLibrary(hModule);
        return 1;
    }

    // Get the base address of the loaded DLL
    DWORD baseAddress = (DWORD)hModule;

    // Parse the headers
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)baseAddress;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("Invalid DOS signature.\n");
        FreeLibrary(hModule);
        return 1;
    }

    IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(baseAddress + dosHeader->e_lfanew);
    if (ntHeader->Signature != IMAGE_NT_SIGNATURE) {
        printf("Invalid NT signature.\n");
        FreeLibrary(hModule);
        return 1;
    }

    IMAGE_OPTIONAL_HEADER* optionalHeader = &ntHeader->OptionalHeader;

    // Resolve imports
    IMAGE_DATA_DIRECTORY* importDirectory = &optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    IMAGE_IMPORT_DESCRIPTOR* importDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)(baseAddress + importDirectory->VirtualAddress);
    while (importDescriptor->Name != 0) {
        char* dllName = (char*)(baseAddress + importDescriptor->Name);
        HMODULE hImportModule = LoadLibraryA(dllName);
        if (hImportModule == NULL) {
            printf("Failed to load library: %s\n", dllName);
            FreeLibrary(hModule);
            return 1;
        }

        IMAGE_THUNK_DATA* importThunk = (IMAGE_THUNK_DATA*)(baseAddress + importDescriptor->OriginalFirstThunk);
        IMAGE_THUNK_DATA* iatEntry = (IMAGE_THUNK_DATA*)(baseAddress + importDescriptor->FirstThunk);

        while (importThunk->u1.AddressOfData != 0) {
            if (IMAGE_SNAP_BY_ORDINAL(importThunk->u1.Ordinal)) {
                printf("Ordinal imports not supported in this example.\n");
                FreeLibrary(hModule);
                return 1;
            }

            IMAGE_IMPORT_BY_NAME* importByName = (IMAGE_IMPORT_BY_NAME*)(baseAddress + importThunk->u1.AddressOfData);
            FARPROC functionAddress = GetProcAddress(hImportModule, importByName->Name);
            if (functionAddress == NULL) {
                printf("Failed to resolve function: %s\n", importByName->Name);
                FreeLibrary(hModule);
                return 1;
            }
            iatEntry->u1.Function = (DWORD)functionAddress;

            importThunk++;
            iatEntry++;
        }

        importDescriptor++;
    }

    // Fix relocations (if necessary) - not implemented in this example

    // Call the desired function from the DLL
    AddFunc addFunc = (AddFunc)(baseAddress + ntHeader->OptionalHeader.AddressOfEntryPoint);
    int sum = addFunc(3, 4);
    printf("Result of adding: %d\n", sum);

    // Cleanup
    result = ((int(*)(HMODULE, DWORD, LPVOID))entryPoint)(hModule, DLL_PROCESS_DETACH, NULL);
    if (result != TRUE) {
        printf("Failed to execute entry point function during cleanup.\n");
    }
    FreeLibrary(hModule);

    return 0;
}
