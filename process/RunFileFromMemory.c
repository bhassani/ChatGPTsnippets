#include <windows.h>
#include <stdio.h>

// Function to read the binary file into memory
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

// Function to execute the code in memory
void ExecuteFromMemory(void* buffer) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)buffer;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)buffer + dosHeader->e_lfanew);

    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        printf("Invalid PE file.\n");
        return;
    }

    // Allocate executable memory
    void* execMemory = VirtualAlloc(NULL, ntHeaders->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!execMemory) {
        printf("Could not allocate executable memory.\n");
        return;
    }

    // Copy headers
    memcpy(execMemory, buffer, ntHeaders->OptionalHeader.SizeOfHeaders);

    // Copy sections
    PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
        memcpy((BYTE*)execMemory + sectionHeader[i].VirtualAddress, 
               (BYTE*)buffer + sectionHeader[i].PointerToRawData, 
               sectionHeader[i].SizeOfRawData);
    }

    // Get entry point
    void (*entryPoint)() = (void (*)())((BYTE*)execMemory + ntHeaders->OptionalHeader.AddressOfEntryPoint);

    // Execute the code
    entryPoint();

    // Free the allocated memory
    VirtualFree(execMemory, 0, MEM_RELEASE);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <executable>\n", argv[0]);
        return 1;
    }

    DWORD filesize;
    void* buffer = ReadFileIntoMemory(argv[1], &filesize);
    if (!buffer) {
        return 1;
    }

    ExecuteFromMemory(buffer);

    VirtualFree(buffer, 0, MEM_RELEASE);
    return 0;
}
