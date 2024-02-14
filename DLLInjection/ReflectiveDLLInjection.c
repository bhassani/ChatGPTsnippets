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

// Function to execute the DLL from memory
BOOL RunDllFromMemory(const char* dllPath) {
    LPVOID lpDllBuffer = MapDllIntoMemory(dllPath);
    if (lpDllBuffer == NULL) {
        return FALSE;
    }

    // Get the reflective loader function from the DLL
    pReflectiveLoader loader = (pReflectiveLoader)((BYTE*)lpDllBuffer + ((PIMAGE_DOS_HEADER)lpDllBuffer)->e_lfanew);
    loader();

    // Free the memory allocated for the DLL buffer
    VirtualFree(lpDllBuffer, 0, MEM_RELEASE);

    return TRUE;
}

int main() {
    const char* dllPath = "YourDLL.dll"; // Specify the path to your DLL here

    if (RunDllFromMemory(dllPath)) {
        printf("DLL executed from memory successfully\n");
    } else {
        printf("Failed to execute DLL from memory\n");
    }

    return 0;
}
