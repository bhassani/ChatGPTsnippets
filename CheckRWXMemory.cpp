#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <unistd.h>

// Function to check for RWX memory regions in a specific process
BOOL HasRWXMemory(DWORD processId) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);

    if (snapshot == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);

    if (!Module32First(snapshot, &me32)) {
        CloseHandle(snapshot);
        return FALSE;
    }

    do {
        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);

        if (processHandle != NULL) {
            MEMORY_BASIC_INFORMATION memInfo;
            VirtualQueryEx(processHandle, me32.modBaseAddr, &memInfo, sizeof(memInfo));

            if (memInfo.State == MEM_COMMIT &&
                (memInfo.Protect == PAGE_EXECUTE_READWRITE ||
                 memInfo.Protect == PAGE_EXECUTE_WRITECOPY ||
                 memInfo.Protect == PAGE_EXECUTE_READ ||
                 memInfo.Protect == PAGE_EXECUTE)) {
                CloseHandle(processHandle);
                CloseHandle(snapshot);
                return TRUE; // RWX memory region detected
            }

            CloseHandle(processHandle);
        }
    } while (Module32Next(snapshot, &me32));

    CloseHandle(snapshot);
    return FALSE;
}

// Function to enumerate running processes and check for RWX memory regions
void CheckProcessesForRWXMemory() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(snapshot, &pe32)) {
        CloseHandle(snapshot);
        return;
    }

    do {
        if (pe32.th32ProcessID != GetCurrentProcessId()) {
            if (HasRWXMemory(pe32.th32ProcessID)) {
                printf("RWX memory region detected in process ID: %lu\n", pe32.th32ProcessID);
                // You may want to take specific actions if RWX memory region is detected in a process
            }
        }
    } while (Process32Next(snapshot, &pe32));

    CloseHandle(snapshot);
}

int main() {
    while (1) {
        CheckProcessesForRWXMemory();

        // Adjust the sleep duration based on your monitoring requirements
        // Sleeping for a short duration to avoid high CPU usage
        Sleep(1000);
    }

    return 0;
}
