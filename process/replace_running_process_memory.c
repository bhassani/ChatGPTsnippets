#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

DWORD GetProcessIdByName(const char *processName) {
    PROCESSENTRY32 pe32;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot failed.\n");
        return 0;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        printf("Process32First failed.\n");
        CloseHandle(hSnapshot);
        return 0;
    }

    do {
        if (strcmp(pe32.szExeFile, processName) == 0) {
            CloseHandle(hSnapshot);
            return pe32.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return 0;
}

int main() {
    const char *targetProcessName = "notepad.exe"; // Replace with the target process name
    DWORD processId = GetProcessIdByName(targetProcessName);

    if (processId == 0) {
        printf("Process not found.\n");
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        printf("OpenProcess failed.\n");
        return 1;
    }

    // Suspend the target process
    HANDLE hThread = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(hThread, &te32)) {
        do {
            if (te32.th32OwnerProcessID == processId) {
                HANDLE hThreadHandle = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                if (hThreadHandle != NULL) {
                    SuspendThread(hThreadHandle);
                    CloseHandle(hThreadHandle);
                }
            }
        } while (Thread32Next(hThread, &te32));
    }
    CloseHandle(hThread);

    // Allocate memory in the target process
    void *remoteMemory = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (remoteMemory == NULL) {
        printf("VirtualAllocEx failed.\n");
        CloseHandle(hProcess);
        return 1;
    }

    // The new executable's path
    const char *newExecutable = "C:\\Path\\To\\Your\\Executable.exe";
    
    // Write the new executable's path to the target process memory
    if (!WriteProcessMemory(hProcess, remoteMemory, newExecutable, strlen(newExecutable) + 1, NULL)) {
        printf("WriteProcessMemory failed.\n");
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // To run the new executable, create a remote thread in the target process that calls CreateProcessA (as an example)
    LPVOID loadLibrary = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibrary, remoteMemory, 0, NULL);
    if (hRemoteThread == NULL) {
        printf("CreateRemoteThread failed.\n");
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    WaitForSingleObject(hRemoteThread, INFINITE);

    // Resume the target process
    hThread = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    te32.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(hThread, &te32)) {
        do {
            if (te32.th32OwnerProcessID == processId) {
                HANDLE hThreadHandle = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                if (hThreadHandle != NULL) {
                    ResumeThread(hThreadHandle);
                    CloseHandle(hThreadHandle);
                }
            }
        } while (Thread32Next(hThread, &te32));
    }

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return 0;
}
