#include <stdio.h>
#include <windows.h>

// Define function pointer type for the functions we want to call from the DLL
typedef int (*AddFunc)(int, int);

int main() {
    // Open the DLL file
    FILE* file = fopen("example.dll", "rb");
    if (file == NULL) {
        printf("Failed to open DLL file.\n");
        return 1;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // Allocate memory to store the DLL content
    unsigned char* buffer = (unsigned char*)malloc(fileSize);
    if (buffer == NULL) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return 1;
    }

    // Read the DLL content into memory
    if (fread(buffer, 1, fileSize, file) != fileSize) {
        printf("Failed to read DLL content.\n");
        fclose(file);
        free(buffer);
        return 1;
    }

    fclose(file);

    // Parse the headers
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)buffer;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("Invalid DOS signature.\n");
        free(buffer);
        return 1;
    }

    IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(buffer + dosHeader->e_lfanew);
    if (ntHeader->Signature != IMAGE_NT_SIGNATURE) {
        printf("Invalid NT signature.\n");
        free(buffer);
        return 1;
    }

    IMAGE_OPTIONAL_HEADER* optionalHeader = &ntHeader->OptionalHeader;

    // Resolve imports
    IMAGE_DATA_DIRECTORY* importDirectory = &optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    IMAGE_IMPORT_DESCRIPTOR* importDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)(buffer + importDirectory->VirtualAddress);
    while (importDescriptor->Name != 0) {
        char* dllName = (char*)(buffer + importDescriptor->Name);
        HMODULE hModule = LoadLibraryA(dllName);
        if (hModule == NULL) {
            printf("Failed to load library: %s\n", dllName);
            free(buffer);
            return 1;
        }

        IMAGE_THUNK_DATA* importThunk = (IMAGE_THUNK_DATA*)(buffer + importDescriptor->OriginalFirstThunk);
        IMAGE_THUNK_DATA* iatEntry = (IMAGE_THUNK_DATA*)(buffer + importDescriptor->FirstThunk);

        while (importThunk->u1.AddressOfData != 0) {
            char* functionName = (char*)(buffer + importThunk->u1.AddressOfData + 2); // Skip the hint
            FARPROC functionAddress = GetProcAddress(hModule, functionName);
            if (functionAddress == NULL) {
                printf("Failed to resolve function: %s\n", functionName);
                free(buffer);
                return 1;
            }
            iatEntry->u1.Function = (DWORD)functionAddress;

            importThunk++;
            iatEntry++;
        }

        importDescriptor++;
    }

    // Perform relocations if necessary (not implemented in this example)

    // Call the entry point function
    DWORD entryPoint = optionalHeader->AddressOfEntryPoint + (DWORD)buffer;
    int result = ((int(*)(void))entryPoint)();

    printf("DLL entry point returned: %d\n", result);

    // Free memory
    free(buffer);

    return 0;
}
