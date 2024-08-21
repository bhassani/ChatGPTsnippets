/*
OpenProcess: This function opens the target process with the necessary permissions to read and write its memory.
EnumProcessModules: Retrieves the base address of the first module loaded by the target process, typically the main executable.
ReadProcessMemory: Reads the DOS and NT headers from the target process's memory to locate the Import Table.
WriteProcessMemory: Zeroes out the Import Table by writing a block of zeroes to the memory location where the Import Table is stored.
Free Resources: Cleans up allocated memory and handles.
*/

#include <windows.h>
#include <stdio.h>

void ZeroProcessImportTable(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
    if (hProcess == NULL) {
        printf("Failed to open process. Error code: %d\n", GetLastError());
        return;
    }

    // Retrieve the base address of the process
    HMODULE hModule = NULL;
    DWORD cbNeeded = 0;
    if (!EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded)) {
        printf("Failed to enumerate process modules. Error code: %d\n", GetLastError());
        CloseHandle(hProcess);
        return;
    }

    // Read the DOS header
    IMAGE_DOS_HEADER dosHeader = { 0 };
    if (!ReadProcessMemory(hProcess, hModule, &dosHeader, sizeof(dosHeader), NULL)) {
        printf("Failed to read DOS header. Error code: %d\n", GetLastError());
        CloseHandle(hProcess);
        return;
    }

    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        printf("Invalid DOS signature.\n");
        CloseHandle(hProcess);
        return;
    }

    // Read the NT headers
    IMAGE_NT_HEADERS ntHeaders = { 0 };
    if (!ReadProcessMemory(hProcess, (LPCVOID)((DWORD_PTR)hModule + dosHeader.e_lfanew), &ntHeaders, sizeof(ntHeaders), NULL)) {
        printf("Failed to read NT headers. Error code: %d\n", GetLastError());
        CloseHandle(hProcess);
        return;
    }

    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
        printf("Invalid NT signature.\n");
        CloseHandle(hProcess);
        return;
    }

    // Locate the import table
    DWORD importTableRVA = ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    DWORD importTableSize = ntHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

    if (importTableRVA == 0) {
        printf("No import table found.\n");
        CloseHandle(hProcess);
        return;
    }

    // Zero out the import table in the process memory
    BYTE *pZeroMemory = (BYTE *)calloc(importTableSize, 1);
    if (!WriteProcessMemory(hProcess, (LPVOID)((DWORD_PTR)hModule + importTableRVA), pZeroMemory, importTableSize, NULL)) {
        printf("Failed to zero out the import table. Error code: %d\n", GetLastError());
    } else {
        printf("Import table zeroed out successfully.\n");
    }

    // Clean up
    free(pZeroMemory);
    CloseHandle(hProcess);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <process_id>\n", argv[0]);
        return 1;
    }

    DWORD pid = (DWORD)atoi(argv[1]);
    ZeroProcessImportTable(pid);
    return 0;
}
