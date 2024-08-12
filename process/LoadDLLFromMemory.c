#include <windows.h>
#include <stdio.h>

// Function to read a file into memory
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
    if (!ReadFile(hFile, buffer, *filesize, &bytesRead, NULL) || bytesRead != *filesize) {
        printf("Error reading the file.\n");
        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hFile);
        return NULL;
    }

    CloseHandle(hFile);
    return buffer;
}

// Function to load the DLL from memory
HMODULE LoadDLLFromMemory(void* buffer, DWORD size) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)buffer;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)buffer + dosHeader->e_lfanew);

    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        printf("Invalid PE file.\n");
        return NULL;
    }

    // Allocate memory for the executable image
    void* execMemory = VirtualAlloc(NULL, ntHeaders->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!execMemory) {
        printf("Could not allocate executable memory.\n");
        return NULL;
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

    // Perform relocations
    if (ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size > 0) {
        PIMAGE_BASE_RELOCATION baseReloc = (PIMAGE_BASE_RELOCATION)((BYTE*)execMemory + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        DWORD oldBase = ntHeaders->OptionalHeader.ImageBase;
        DWORD newBase = (DWORD)execMemory;
        DWORD delta = newBase - oldBase;

        while (baseReloc->SizeOfBlock > 0) {
            WORD* relocData = (WORD*)((BYTE*)baseReloc + sizeof(IMAGE_BASE_RELOCATION));
            for (int i = 0; i < (baseReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD); i++) {
                if (relocData[i] & IMAGE_REL_BASED_HIGHLOW) {
                    DWORD* relocAddress = (DWORD*)((BYTE*)execMemory + baseReloc->VirtualAddress + (relocData[i] & 0x0FFF));
                    *relocAddress += delta;
                }
            }
            baseReloc = (PIMAGE_BASE_RELOCATION)((BYTE*)baseReloc + baseReloc->SizeOfBlock);
        }
    }

    // Resolve imports
    // Import resolution code should be added here for a complete implementation.

    // Get entry point
    void (*entryPoint)() = (void (*)())((BYTE*)execMemory + ntHeaders->OptionalHeader.AddressOfEntryPoint);

    // Execute the code
    entryPoint();

    return (HMODULE)execMemory;
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

    LoadDLLFromMemory(buffer, filesize);

    VirtualFree(buffer, 0, MEM_RELEASE);
    return 0;
}
