#include <windows.h>
#include <detours.h>
#include <stdio.h>
#include <string.h>

// Define the function pointers for the original DNS API functions
typedef DNS_STATUS (WINAPI *DnsQuery_A_t)(LPCSTR, DNS_TYPE, DWORD, PVOID, PVOID*, PVOID);
typedef DNS_STATUS (WINAPI *DnsQuery_UTF8_t)(LPCSTR, DNS_TYPE, DWORD, PVOID, PVOID*, PVOID);
typedef DNS_STATUS (WINAPI *DnsQuery_W_t)(LPCWSTR, DNS_TYPE, DWORD, PVOID, PVOID*, PVOID);
typedef DNS_STATUS (WINAPI *Query_Main_t)(LPCWSTR, DWORD, PVOID, PVOID*, PVOID);

// Function pointers for original functions
static DnsQuery_A_t Real_DnsQuery_A = DnsQuery_A;
static DnsQuery_UTF8_t Real_DnsQuery_UTF8 = DnsQuery_UTF8;
static DnsQuery_W_t Real_DnsQuery_W = DnsQuery_W;
static Query_Main_t Real_Query_Main = Query_Main;

// Blocked websites
const char* blocked_websites[] = {
    "www.badwebsite.com",
    "www.anothersite.com",
};
const int num_blocked_websites = sizeof(blocked_websites) / sizeof(blocked_websites[0]);

// Function to check if a URL is in the blocked list
int is_website_blocked(const char* url) {
    for (int i = 0; i < num_blocked_websites; ++i) {
        if (strstr(url, blocked_websites[i]) != NULL) {
            return 1;
        }
    }
    return 0;
}

// Hooked DNS API functions
DNS_STATUS WINAPI Hooked_DnsQuery_A(
    LPCSTR pszName,
    DNS_TYPE wType,
    DWORD Options,
    PVOID pQueryResults,
    PVOID* ppQueryResults,
    PVOID pReserved
) {
    // Check if the requested URL is blocked
    if (is_website_blocked(pszName)) {
        SetLastError(ERROR_TIMEOUT);
        return DNS_REQUEST_PENDING; // Simulate timeout
    }
    // Call the original function
    return Real_DnsQuery_A(pszName, wType, Options, pQueryResults, ppQueryResults, pReserved);
}

DNS_STATUS WINAPI Hooked_DnsQuery_UTF8(
    LPCSTR pszName,
    DNS_TYPE wType,
    DWORD Options,
    PVOID pQueryResults,
    PVOID* ppQueryResults,
    PVOID pReserved
) {
    // Check if the requested URL is blocked
    if (is_website_blocked(pszName)) {
        SetLastError(ERROR_TIMEOUT);
        return DNS_REQUEST_PENDING; // Simulate timeout
    }
    // Call the original function
    return Real_DnsQuery_UTF8(pszName, wType, Options, pQueryResults, ppQueryResults, pReserved);
}

DNS_STATUS WINAPI Hooked_DnsQuery_W(
    LPCWSTR pszName,
    DNS_TYPE wType,
    DWORD Options,
    PVOID pQueryResults,
    PVOID* ppQueryResults,
    PVOID pReserved
) {
    // Convert wide string to narrow string for blocked website check
    char name[256];
    WideCharToMultiByte(CP_ACP, 0, pszName, -1, name, sizeof(name), NULL, NULL);

    // Check if the requested URL is blocked
    if (is_website_blocked(name)) {
        SetLastError(ERROR_TIMEOUT);
        return DNS_REQUEST_PENDING; // Simulate timeout
    }
    // Call the original function
    return Real_DnsQuery_W(pszName, wType, Options, pQueryResults, ppQueryResults, pReserved);
}

DNS_STATUS WINAPI Hooked_Query_Main(
    LPCWSTR pszName,
    DWORD Options,
    PVOID pQueryResults,
    PVOID* ppQueryResults,
    PVOID pReserved
) {
    // Convert wide string to narrow string for blocked website check
    char name[256];
    WideCharToMultiByte(CP_ACP, 0, pszName, -1, name, sizeof(name), NULL, NULL);

    // Check if the requested URL is blocked
    if (is_website_blocked(name)) {
        SetLastError(ERROR_TIMEOUT);
        return DNS_REQUEST_PENDING; // Simulate timeout
    }
    // Call the original function
    return Real_Query_Main(pszName, Options, pQueryResults, ppQueryResults, pReserved);
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // Attach hooks
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)Real_DnsQuery_A, Hooked_DnsQuery_A);
        DetourAttach(&(PVOID&)Real_DnsQuery_UTF8, Hooked_DnsQuery_UTF8);
        DetourAttach(&(PVOID&)Real_DnsQuery_W, Hooked_DnsQuery_W);
        DetourAttach(&(PVOID&)Real_Query_Main, Hooked_Query_Main);
        DetourTransactionCommit();
        break;
    case DLL_PROCESS_DETACH:
        // Detach hooks
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)Real_DnsQuery_A, Hooked_DnsQuery_A);
        DetourDetach(&(PVOID&)Real_DnsQuery_UTF8, Hooked_DnsQuery_UTF8);
        DetourDetach(&(PVOID&)Real_DnsQuery_W, Hooked_DnsQuery_W);
        DetourDetach(&(PVOID&)Real_Query_Main, Hooked_Query_Main);
        DetourTransactionCommit();
        break;
    }
    return TRUE;
}
