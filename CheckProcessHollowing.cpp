#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <unistd.h>

// Function to check for process hollowing in a specific process
BOOL IsProcessHollowed(DWORD processId) {
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
        // Compare the base address of the main module with the base address of other modules
        if (me32.modBaseAddr != me32.hModule) {
            CloseHandle(snapshot);
            return TRUE; // Process hollowing detected
        }
    } while (Module32Next(snapshot, &me32));

    CloseHandle(snapshot);
    return FALSE;
}

// Function to enumerate running processes and check for process hollowing
void CheckProcessesForHollowing() {
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
            if (IsProcessHollowed(pe32.th32ProcessID)) {
                printf("Process hollowing detected in process ID: %lu\n", pe32.th32ProcessID);
                // You may want to take specific actions if hollowing is detected in a process
            }
        }
    } while (Process32Next(snapshot, &pe32));

    CloseHandle(snapshot);
}

int main() {
    while (1) {
        CheckProcessesForHollowing();

        // Adjust the sleep duration based on your monitoring requirements
        // Sleeping for a short duration to avoid high CPU usage
        Sleep(1000);
    }

    return 0;
}
