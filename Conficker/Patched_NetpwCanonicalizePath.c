#include <windows.h>
#include <detours.h>
#include <stdio.h>
#include <string.h>

// Define the type for the NetpwCanonicalizePath function
typedef NET_API_STATUS (WINAPI *NetpwCanonicalizePath_t)(LPWSTR, LPWSTR*);

// Declare a pointer to the original function
NetpwCanonicalizePath_t OriginalNetpwCanonicalizePath = NULL;

// Define the hook function
NET_API_STATUS WINAPI HookedNetpwCanonicalizePath(LPWSTR Path, LPWSTR *CanonicalizedPath) {
    // Check if Path is valid
    if (Path != NULL) {
        // Convert WCHAR string to a regular string for easier substring search
        char pathBuffer[2048]; // Buffer for conversion
        size_t length = wcslen(Path) * sizeof(WCHAR);
        if (length > sizeof(pathBuffer)) {
            length = sizeof(pathBuffer) - 1;
        }

        // Convert wide string to multi-byte string
        WideCharToMultiByte(CP_ACP, 0, Path, -1, pathBuffer, (int)length, NULL, NULL);

        // Check if the path length exceeds 200 bytes
        if (length > 200) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return ERROR_INVALID_PARAMETER;
        }

        // Check if the path contains the substring "\..\"
        if (strstr(pathBuffer, "\\..\\") != NULL) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return ERROR_INVALID_PARAMETER;
        }
    }

    // Call the original function if checks pass
    return OriginalNetpwCanonicalizePath(Path, CanonicalizedPath);
}

// Main function to setup and run the hook
int main() {
    // Load the netapi32.dll library
    HMODULE hNetapi32 = LoadLibrary(L"netapi32.dll");
    if (hNetapi32 == NULL) {
        printf("Failed to load netapi32.dll\n");
        return 1;
    }

    // Get the address of the NetpwCanonicalizePath function
    OriginalNetpwCanonicalizePath = (NetpwCanonicalizePath_t)GetProcAddress(hNetapi32, "NetpwCanonicalizePath");
    if (OriginalNetpwCanonicalizePath == NULL) {
        printf("Failed to get address of NetpwCanonicalizePath\n");
        return 1;
    }

    // Attach the hook using Detours
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)OriginalNetpwCanonicalizePath, HookedNetpwCanonicalizePath);
    DetourTransactionCommit();

    printf("Hook installed. Press Enter to exit...\n");
    getchar(); // Wait for user input to exit

    // Detach the hook when done
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)OriginalNetpwCanonicalizePath, HookedNetpwCanonicalizePath);
    DetourTransactionCommit();

    // Free the loaded library
    FreeLibrary(hNetapi32);

    return 0;
}
