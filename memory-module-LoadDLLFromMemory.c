#include <windows.h>
#include "memory_module.h"  // Replace with the actual path to the memory-module header file
#include <stdio.h>

void* ReadFileIntoMemory(const char* filename, DWORD* filesize) {
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Could not open file %s.\n", filename);
        return NULL;
    }

    *filesize = GetFileSize(hFile, NULL);
    void* buffer = VirtualAlloc(NULL, *filesize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!buffer) {
        printf("Could not allocate memory.\n");
        CloseHandle(hFile);
        return NULL;
    }

    DWORD bytesRead;
    ReadFile(hFile, buffer, *filesize, &bytesRead, NULL);
    CloseHandle(hFile);

    if (bytesRead != *filesize) {
        printf("Error reading the file.\n");
        VirtualFree(buffer, 0, MEM_RELEASE);
        return NULL;
    }

    return buffer;
}


void LoadAndExecuteDLLFromMemory(void* buffer, DWORD size) {
    // Initialize the memory module with the DLL buffer
    HMEMORYMODULE hMod = MemoryLoadLibrary(buffer, size);
    if (!hMod) {
        printf("Failed to load DLL from memory.\n");
        return;
    }

    // Optionally, you can get the address of exported functions and use them
    FARPROC pFunc = MemoryGetProcAddress(hMod, "YourFunctionName");
    if (pFunc) {
        // Call the function if needed
        pFunc();
    } else {
        printf("Function not found.\n");
    }

    // Free the memory module when done
    MemoryFreeLibrary(hMod);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <dll-file>\n", argv[0]);
        return 1;
    }

    DWORD filesize;
    void* buffer = ReadFileIntoMemory(argv[1], &filesize);
    if (!buffer) {
        return 1;
    }

    LoadAndExecuteDLLFromMemory(buffer, filesize);

    VirtualFree(buffer, 0, MEM_RELEASE);
    return 0;
}
