#include <windows.h>
#include <detours.h>

typedef NTSTATUS(WINAPI* PNetpwCanonicalizePath)(
    LPCWSTR pszServer,
    LPCWSTR pszPath,
    LPWSTR pszCanonicalizedPath,
    ULONG cchCanonicalizedPath
);

PNetpwCanonicalizePath pRealNetpwCanonicalizePath = NULL;

NTSTATUS WINAPI DetourNetpwCanonicalizePath(
    LPCWSTR pszServer,
    LPCWSTR pszPath,
    LPWSTR pszCanonicalizedPath,
    ULONG cchCanonicalizedPath
) {
    // Your custom code or modifications go here

    // Call the original function
    return pRealNetpwCanonicalizePath(pszServer, pszPath, pszCanonicalizedPath, cchCanonicalizedPath);
}

int main() {
    // Initialize Detours
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pRealNetpwCanonicalizePath, DetourNetpwCanonicalizePath);
    DetourTransactionCommit();

    // Your program logic goes here

    // Cleanup Detours
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)pRealNetpwCanonicalizePath, DetourNetpwCanonicalizePath);
    DetourTransactionCommit();

    return 0;
}

