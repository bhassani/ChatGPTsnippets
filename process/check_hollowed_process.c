#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

// Get the module path of a process by its ID
char* GetModulePath(DWORD pid) {
    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;
    char* modulePath = NULL;

    // Take a snapshot of all modules in the specified process
    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (hModuleSnap == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot failed. Error: %d\n", GetLastError());
        return NULL;
    }

    // Set the size of the structure before using it
    me32.dwSize = sizeof(MODULEENTRY32);

    // Retrieve information about the first module
    if (!Module32First(hModuleSnap, &me32)) {
        printf("Module32First failed. Error: %d\n", GetLastError());
        CloseHandle(hModuleSnap); // Clean the snapshot object
        return NULL;
    }

    // Allocate memory for the module path and copy it
    modulePath = (char*)malloc(MAX_PATH);
    strcpy(modulePath, me32.szExePath);

    // Close the handle to the snapshot object
    CloseHandle(hModuleSnap);
    return modulePath;
}

// Check if a process is hollowed by comparing its main module to its module path
BOOL IsProcessHollowed(DWORD pid) {
    HANDLE hProcess = NULL;
    char* modulePath = NULL;
    char* mainModule = NULL;
    BOOL result = FALSE;

    // Open the process with read access
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess == NULL) {
        printf("OpenProcess failed. Error: %d\n", GetLastError());
        return FALSE;
    }

    // Get the module path of the process
    modulePath = GetModulePath(pid);
    if (modulePath == NULL) {
        printf("GetModulePath failed.\n");
        CloseHandle(hProcess);
        return FALSE;
    }

    // Allocate memory for the main module and read it from the process
    mainModule = (char*)malloc(MAX_PATH);
    if (!GetModuleFileNameEx(hProcess, NULL, mainModule, MAX_PATH)) {
        printf("GetModuleFileNameEx failed. Error: %d\n", GetLastError());
        free(modulePath);
        free(mainModule);
        CloseHandle(hProcess);
        return FALSE;
    }

    // Compare the module path and the main module
    if (strcmp(modulePath, mainModule) != 0) {
        result = TRUE; // The process is hollowed
    }

    // Free the allocated memory and close the handle
    free(modulePath);
    free(mainModule);
    CloseHandle(hProcess);
    return result;
}

// Print the hollowed processes on the system
void PrintHollowedProcesses() {
    HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe32;

    // Take a snapshot of all processes in the system
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot failed. Error: %d\n", GetLastError());
        return;
    }

    // Set the size of the structure before using it
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process
    if (!Process32First(hProcessSnap, &pe32)) {
        printf("Process32First failed. Error: %d\n", GetLastError());
        CloseHandle(hProcessSnap); // Clean the snapshot object
        return;
    }

    // Loop through all processes
    do {
        // Check if the process is hollowed
        if (IsProcessHollowed(pe32.th32ProcessID)) {
            // Print the process name and ID
            printf("Hollowed process: %s (PID: %d)\n", pe32.szExeFile, pe32.th32ProcessID);
        }
    } while (Process32Next(hProcessSnap, &pe32)); // Get the next process

    // Close the handle to the snapshot object
    CloseHandle(hProcessSnap);
}

// Main function
int main() {
    // Print the hollowed processes on the system
    PrintHollowedProcesses();
    return 0;
}
