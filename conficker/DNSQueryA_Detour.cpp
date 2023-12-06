/*
Idea from conficker:

Using Microsoft Detours to intercept the DNSQuery_A function involves hooking into the DNS client library and redirecting calls to your custom implementation. 

Below is a basic example demonstrating how you can use Detours to intercept DNSQuery_A on Windows. 

Note that modifying system functions should be done with caution, and this example is for educational purposes only.

links: https://www.codeproject.com/Articles/30140/API-Hooking-with-MS-Detours
https://www.codeproject.com/KB/system/KamalDetours01/
*/

#include <Windows.h>
#include <Detours.h> //#include <detours\detours.h>
#include <Dnsapi.h>
#pragma comment(lib, "detours.lib")

/*
We define a custom function MyDNSQuery_A, which will be our replacement for DNSQuery_A.
We use Detours to intercept DNSQuery_A and redirect it to MyDNSQuery_A.
Inside MyDNSQuery_A, you can implement your custom logic. In this case, it simply prints a message and then calls the original DNSQuery_A function.
Make sure to link your project against the Detours library and include the Detours header. 
*/

// Function pointer type for the original DNSQuery_A function
typedef DNS_STATUS(WINAPI* DNSQuery_A_t)(
    PCSTR           pszName,
    WORD            wType,
    DWORD           Options,
    PIP4_ARRAY      aipServers,
    PDNS_RECORDA*   ppQueryResultsSet,
    PVOID*          pReserved
);

// Original DNSQuery_A function
DNSQuery_A_t OriginalDNSQuery_A = nullptr;

// Custom implementation of DNSQuery_A
DNS_STATUS WINAPI MyDNSQuery_A(
    PCSTR           pszName,
    WORD            wType,
    DWORD           Options,
    PIP4_ARRAY      aipServers,
    PDNS_RECORDA*   ppQueryResultsSet,
    PVOID*          pReserved
) {
    // Your custom implementation here
    printf("DNSQuery_A intercepted!\n");

    // Call the original DNSQuery_A function
    return OriginalDNSQuery_A(pszName, wType, Options, aipServers, ppQueryResultsSet, pReserved);
}

int main() {
    // Initialize Detours
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // Get a pointer to the original DNSQuery_A function
    OriginalDNSQuery_A = reinterpret_cast<DNSQuery_A_t>(GetProcAddress(GetModuleHandle(L"dnsapi.dll"), "DNSQuery_A"));

    // Attach Detours to the DNSQuery_A function
    DetourAttach(reinterpret_cast<PVOID*>(&OriginalDNSQuery_A), MyDNSQuery_A);

    // Finalize Detours
    DetourTransactionCommit();

    // Test DNSQuery_A
    DNSQuery_A("example.com", DNS_TYPE_A, 0, nullptr, nullptr, nullptr);

    // Detach Detours before exiting
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(reinterpret_cast<PVOID*>(&OriginalDNSQuery_A), MyDNSQuery_A);
    DetourTransactionCommit();

    return 0;
}





