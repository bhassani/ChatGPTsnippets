#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    // Get network adapter information using GetAdaptersAddresses
    ULONG bufferSize = 0;
    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &bufferSize) != ERROR_BUFFER_OVERFLOW) {
        fprintf(stderr, "Error getting buffer size for GetAdaptersAddresses\n");
        WSACleanup();
        return 1;
    }

    PIP_ADAPTER_ADDRESSES adapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(bufferSize);
    if (adapterAddresses == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        WSACleanup();
        return 1;
    }

    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, adapterAddresses, &bufferSize) != NO_ERROR) {
        fprintf(stderr, "Error getting adapter addresses\n");
        free(adapterAddresses);
        WSACleanup();
        return 1;
    }

    // Iterate through the adapters
    PIP_ADAPTER_ADDRESSES pAdapter = adapterAddresses;
    while (pAdapter) {
        // Display the current IP address
        printf("Original IP Address: %s\n", pAdapter->FirstUnicastAddress->Address.lpSockaddr->sa_data + 2);

        // Change the last two octets to random numbers
        PBYTE ipBytes = (PBYTE)&pAdapter->FirstUnicastAddress->Address.lpSockaddr->sa_data[2];
        ipBytes[2] = rand() % 256;
        ipBytes[3] = rand() % 256;

        // Display the modified IP address
        printf("Modified IP Address: %d.%d.%d.%d\n", ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]);

        // Move to the next adapter
        pAdapter = pAdapter->Next;
    }

    // Cleanup
    free(adapterAddresses);
    WSACleanup();

    return 0;
}
