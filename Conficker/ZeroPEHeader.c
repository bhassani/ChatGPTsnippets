#include <windows.h>
#include <stdio.h>

void ZeroImportTable(LPCSTR filePath) {
    HANDLE hFile = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Failed to open file.\n");
        return;
    }

    HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (!hMapping) {
        printf("Failed to create file mapping.\n");
        CloseHandle(hFile);
        return;
    }

    LPVOID pFileBase = MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
    if (!pFileBase) {
        printf("Failed to map view of file.\n");
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)pFileBase;
    if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("Invalid DOS header.\n");
        UnmapViewOfFile(pFileBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)pFileBase + pDOSHeader->e_lfanew);
    if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE) {
        printf("Invalid NT headers.\n");
        UnmapViewOfFile(pFileBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    // Locate the import table
    PIMAGE_DATA_DIRECTORY pImportTableDirectory = &pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (pImportTableDirectory->VirtualAddress == 0) {
        printf("No import table found.\n");
    } else {
        // Zero out the import table
        ZeroMemory((LPVOID)((DWORD_PTR)pFileBase + pImportTableDirectory->VirtualAddress), pImportTableDirectory->Size);
        printf("Import table zeroed out.\n");
    }

    // Clean up
    UnmapViewOfFile(pFileBase);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <executable_path>\n", argv[0]);
        return 1;
    }

    ZeroImportTable(argv[1]);
    return 0;
}
