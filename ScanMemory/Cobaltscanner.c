#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

#define TARGET_PATTERN "\x49\xB9\x01\x01\x01\x01\x01\x01\x01\x01\x49\x0F\xAF\xD1\x49\x83\xF8\x40"
#define PATTERN_SIZE 18

BOOL SearchPatternInMemory(HANDLE hProcess, LPCVOID baseAddress, SIZE_T regionSize) {
    BYTE* buffer = (BYTE*)malloc(regionSize);
    if (!buffer) {
        return FALSE;
    }

    SIZE_T bytesRead;
    if (ReadProcessMemory(hProcess, baseAddress, buffer, regionSize, &bytesRead)) {
        for (SIZE_T i = 0; i < bytesRead - PATTERN_SIZE; i++) {
            if (memcmp(buffer + i, TARGET_PATTERN, PATTERN_SIZE) == 0) {
                free(buffer);
                return TRUE;
            }
        }
    }

    free(buffer);
    return FALSE;
}

void ScanProcess(DWORD processID) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (hProcess == NULL) {
        return;
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    LPCVOID baseAddress = sysInfo.lpMinimumApplicationAddress;
    LPCVOID maxAddress = sysInfo.lpMaximumApplicationAddress;

    MEMORY_BASIC_INFORMATION mbi;

    while (baseAddress < maxAddress) {
        if (VirtualQueryEx(hProcess, baseAddress, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            if (mbi.State == MEM_COMMIT && (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE)) {
                if (SearchPatternInMemory(hProcess, baseAddress, mbi.RegionSize)) {
                    printf("Found in process %lu at address %p\n", processID, baseAddress);
                }
            }
            baseAddress = (LPCVOID)((SIZE_T)baseAddress + mbi.RegionSize);
        } else {
            break;
        }
    }

    CloseHandle(hProcess);
}

int main() {
    while (1) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            return 1;
        }

        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hSnapshot, &pe32)) {
            do {
                // Skip system processes
                if (_wcsicmp(pe32.szExeFile, L"System") != 0 && _wcsicmp(pe32.szExeFile, L"smss.exe") != 0) {
                    ScanProcess(pe32.th32ProcessID);
                }
            } while (Process32Next(hSnapshot, &pe32));
        }

        CloseHandle(hSnapshot);
        Sleep(5000);  // Sleep for 5 seconds before the next scan
    }

    return 0;
}
