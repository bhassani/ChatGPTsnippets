#include <windows.h>
#include <wininet.h>
#include <stdio.h>

// Function to parse the HTTP response and extract the date
BOOL ParseHTTPResponse(HINTERNET hRequest, SYSTEMTIME *lpSystemTime) {
    BOOL bSuccess = FALSE;
    CHAR szBuffer[1024];
    DWORD dwSize = sizeof(szBuffer);
    DWORD dwDownloaded = 0;
    
    // Read the HTTP response
    if (!InternetReadFile(hRequest, szBuffer, dwSize, &dwDownloaded) || dwDownloaded == 0) {
        printf("Failed to read HTTP response\n");
        return FALSE;
    }
    
    // Null-terminate the response buffer
    szBuffer[dwDownloaded] = '\0';

    // Search for the "Date:" header in the response
    const char* dateHeader = "Date:";
    char* pDateStart = strstr(szBuffer, dateHeader);
    if (pDateStart != NULL) {
        pDateStart += strlen(dateHeader);
        // Skip leading spaces
        while (*pDateStart == ' ' || *pDateStart == '\t') {
            pDateStart++;
        }

        // Parse the date and time
        if (sscanf(pDateStart, "%*3s, %d %*3s %d %d:%d:%d", &lpSystemTime->wDayOfWeek,
            &lpSystemTime->wDay, &lpSystemTime->wYear, &lpSystemTime->wHour,
            &lpSystemTime->wMinute, &lpSystemTime->wSecond) == 6) {
            bSuccess = TRUE;
        }
    }

    return bSuccess;
}

int main() {
    HINTERNET hInternet = InternetOpen("HTTP Request", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL) {
        printf("Failed to initialize WinINet\n");
        return 1;
    }

    HINTERNET hConnect = InternetOpenUrl(hInternet, "http://www.example.com", NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hConnect == NULL) {
        printf("Failed to open URL\n");
        InternetCloseHandle(hInternet);
        return 1;
    }

    SYSTEMTIME systemTime;
    if (ParseHTTPResponse(hConnect, &systemTime)) {
        printf("Date and time: %04d-%02d-%02d %02d:%02d:%02d\n",
               systemTime.wYear, systemTime.wMonth, systemTime.wDay,
               systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
    } else {
        printf("Failed to parse HTTP response\n");
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return 0;
}
