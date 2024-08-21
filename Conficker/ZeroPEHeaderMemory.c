#include <windows.h>
#include <stdio.h>

void ZeroProcessPEHeader(DWORD pid) {
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

    // Read the DOS header to find the size of the headers
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

    // Calculate the total size of the PE headers (DOS header + NT headers + optional headers + section headers)
    DWORD headerSize = dosHeader.e_lfanew + sizeof(IMAGE_NT_HEADERS) + (ntHeaders.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));

    // Zero out the PE header in the process memory
    BYTE *pZeroMemory = (BYTE *)calloc(headerSize, 1);
    if (!WriteProcessMemory(hProcess, hModule, pZeroMemory, headerSize, NULL)) {
        printf("Failed to zero out the PE header. Error code: %d\n", GetLastError());
    } else {
        printf("PE header zeroed out successfully.\n");
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
    ZeroProcessPEHeader(pid);
    return 0;
}
