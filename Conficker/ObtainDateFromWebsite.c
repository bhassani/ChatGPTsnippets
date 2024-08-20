#include <windows.h>
#include <wininet.h>
#include <stdio.h>

#pragma comment(lib, "wininet.lib")

void PrintSystemTime(const SYSTEMTIME *st) {
    printf("Date: %04d-%02d-%02d %02d:%02d:%02d\n",
           st->wYear, st->wMonth, st->wDay,
           st->wHour, st->wMinute, st->wSecond);
}

int main() {
    HINTERNET hInternet, hConnect, hRequest;
    BOOL bSuccess;
    DWORD dwSize;
    char szDate[64];
    SYSTEMTIME systemTime;

    // Step 1: Initialize WinInet
    hInternet = InternetOpen("HttpQueryInfo Example", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hInternet == NULL) {
        printf("InternetOpen failed. Error: %lu\n", GetLastError());
        return 1;
    }

    // Step 2: Open a Connection
    hConnect = InternetConnect(hInternet, "www.baidu.com", INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (hConnect == NULL) {
        printf("InternetConnect failed. Error: %lu\n", GetLastError());
        InternetCloseHandle(hInternet);
        return 1;
    }

    // Step 3: Open a Request
    hRequest = HttpOpenRequest(hConnect, "HEAD", "/", NULL, NULL, NULL, INTERNET_FLAG_RELOAD, 0);
    if (hRequest == NULL) {
        printf("HttpOpenRequest failed. Error: %lu\n", GetLastError());
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return 1;
    }

    // Step 4: Send the Request
    bSuccess = HttpSendRequest(hRequest, NULL, 0, NULL, 0);
    if (!bSuccess) {
        printf("HttpSendRequest failed. Error: %lu\n", GetLastError());
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return 1;
    }

    // Step 5: Query the Date Header
    dwSize = sizeof(szDate);
    bSuccess = HttpQueryInfo(hRequest, HTTP_QUERY_DATE, &szDate, &dwSize, NULL);
    if (!bSuccess) {
        printf("HttpQueryInfo failed. Error: %lu\n", GetLastError());
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return 1;
    }

    printf("Date header: %s\n", szDate);

    // Step 6: Convert the Date to SYSTEMTIME
    if (InternetTimeToSystemTime(szDate, &systemTime, 0)) {
        PrintSystemTime(&systemTime);
    } else {
        printf("InternetTimeToSystemTime failed. Error: %lu\n", GetLastError());
    }

    // Clean up
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return 0;
}
