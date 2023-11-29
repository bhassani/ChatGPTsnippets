#include <stdio.h>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

void getPublicIP() {
    HINTERNET hInternet, hConnect;
    char buffer[1024];
    DWORD bytesRead;

    // Initialize WinINet
    hInternet = InternetOpen(L"IP Retrieval", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL) {
        fprintf(stderr, "InternetOpen failed\n");
        return;
    }

    // Open a connection to the service that returns the public IP address
    hConnect = InternetOpenUrlA(hInternet, "https://api64.ipify.org?format=json", NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hConnect == NULL) {
        fprintf(stderr, "InternetOpenUrlA failed\n");
        InternetCloseHandle(hInternet);
        return;
    }

    // Read the response
    if (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead)) {
        buffer[bytesRead] = '\0';
        printf("Public IP Address: %s\n", buffer);
    } else {
        fprintf(stderr, "InternetReadFile failed\n");
    }

    // Clean up
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
}

int main() {
    getPublicIP();

    return 0;
}
