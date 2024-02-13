#include <windows.h>
#include <stdio.h>

//This code first loads each DLL using LoadLibraryA(), then it parses the PE header to locate the import directory. 
//It iterates over each imported module and function, printing out their names or ordinals. 
//Finally, it frees the loaded DLL using FreeLibrary(). You need to link against kernel32.lib to use LoadLibraryA() and FreeLibrary().

void RebuildIAT(const char* dllPath) {
    // Load the DLL
    HMODULE hModule = LoadLibraryA(dllPath);
    if (hModule == NULL) {
        printf("Failed to load DLL: %s\n", dllPath);
        return;
    }

    // Get the address of the import directory
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hModule +
        ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    // Loop through import descriptors
    while (importDesc->Name) {
        // Get the module name
        const char* moduleName = (const char*)((BYTE*)hModule + importDesc->Name);
        printf("Module: %s\n", moduleName);

        // Get the IAT entries
        PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((BYTE*)hModule + importDesc->OriginalFirstThunk);
        PIMAGE_THUNK_DATA iat = (PIMAGE_THUNK_DATA)((BYTE*)hModule + importDesc->FirstThunk);

        // Iterate through each imported function
        while (thunk->u1.AddressOfData) {
            // Check if the function is imported by ordinal
            if (IMAGE_SNAP_BY_ORDINAL(thunk->u1.Ordinal)) {
                printf("\tOrdinal: %ld\n", IMAGE_ORDINAL(thunk->u1.Ordinal));
            }
            else {
                PIMAGE_IMPORT_BY_NAME importByName = (PIMAGE_IMPORT_BY_NAME)((BYTE*)hModule + thunk->u1.AddressOfData);
                printf("\tFunction: %s\n", importByName->Name);
            }

            // Move to the next thunk
            ++thunk;
            ++iat;
        }

        // Move to the next import descriptor
        ++importDesc;
    }

    // Free the loaded DLL
    FreeLibrary(hModule);
}

int main() {
    // Paths to the DLLs
    const char* dllPaths[] = {
        "C:\\Windows\\System32\\ntdll.dll",
        "C:\\Windows\\System32\\kernel32.dll",
        "C:\\Windows\\System32\\advapi32.dll",
        "C:\\Windows\\System32\\user32.dll"
    };

    // Rebuild the IAT for each DLL
    for (int i = 0; i < sizeof(dllPaths) / sizeof(dllPaths[0]); ++i) {
        printf("Rebuilding IAT for: %s\n", dllPaths[i]);
        RebuildIAT(dllPaths[i]);
    }

    return 0;
}
