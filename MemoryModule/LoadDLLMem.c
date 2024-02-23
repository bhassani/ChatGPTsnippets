#include <stdio.h>
#include "MemoryModule.h"

int main() {
    // Load the DLL file into memory
    FILE* file = fopen("example.dll", "rb");
    if (file == NULL) {
        printf("Failed to open DLL file.\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    unsigned char* buffer = (unsigned char*)malloc(fileSize);
    if (buffer == NULL) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return 1;
    }

    if (fread(buffer, 1, fileSize, file) != fileSize) {
        printf("Failed to read DLL content.\n");
        fclose(file);
        free(buffer);
        return 1;
    }

    fclose(file);

    // Load the DLL from memory
    HMEMORYMODULE hModule = MemoryLoadLibrary(buffer, fileSize);
    if (hModule == NULL) {
        printf("Failed to load DLL from memory.\n");
        free(buffer);
        return 1;
    }

    // Get a pointer to the entry point function
    FARPROC entryPoint = MemoryGetProcAddress(hModule, "DllMain");
    if (entryPoint == NULL) {
        printf("Failed to get entry point address.\n");
        MemoryFreeLibrary(hModule);
        free(buffer);
        return 1;
    }

    // Call the entry point function
    int result = ((int(*)(HMODULE, DWORD, LPVOID))entryPoint)((HMODULE)hModule, DLL_PROCESS_ATTACH, NULL);
    if (result != TRUE) {
        printf("Failed to execute entry point function.\n");
        MemoryFreeLibrary(hModule);
        free(buffer);
        return 1;
    }

    // Perform necessary operations with the loaded DLL...

    // Free the DLL from memory
    MemoryFreeLibrary(hModule);
    free(buffer);

    return 0;
}
