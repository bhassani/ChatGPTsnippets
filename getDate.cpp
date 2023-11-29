#include <stdio.h>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")
#include <time.h>

int convert_time() {
    const char *dateString = "2023-11-15 12:30:45";
    struct tm datetime;

    // Use strptime to parse the string and fill the tm structure
    if (strptime(dateString, "%Y-%m-%d %H:%M:%S", &datetime) == NULL) {
        fprintf(stderr, "Failed to parse date string\n");
        return 1;
    }

    // You can now access individual components of the datetime structure
    printf("Year: %d\n", datetime.tm_year + 1900);
    printf("Month: %d\n", datetime.tm_mon + 1);
    printf("Day: %d\n", datetime.tm_mday);
    printf("Hour: %d\n", datetime.tm_hour);
    printf("Minute: %d\n", datetime.tm_min);
    printf("Second: %d\n", datetime.tm_sec);

    return 0;
}

int convert_time_to_systemtime() {
    const char *dateString = "2023-11-15 12:30:45";
    SYSTEMTIME sysTime;
    FILETIME fileTime;

    // Parse the date string and fill the SYSTEMTIME structure
    sscanf(dateString, "%d-%d-%d %d:%d:%d",
           &sysTime.wYear, &sysTime.wMonth, &sysTime.wDay,
           &sysTime.wHour, &sysTime.wMinute, &sysTime.wSecond);

    // Convert SYSTEMTIME to FILETIME
    if (!SystemTimeToFileTime(&sysTime, &fileTime)) {
        fprintf(stderr, "Failed to convert SYSTEMTIME to FILETIME\n");
        return 1;
    }

    // Now you can use the fileTime structure as needed

    return 0;
}

void getDateFromHttpResponse() {
    HINTERNET hInternet, hConnect;
    char buffer[1024];
    DWORD bytesRead;
    DWORD bufferSize = sizeof(buffer);

    // Initialize WinINet
    hInternet = InternetOpen(L"HTTP Date Retrieval", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL) {
        fprintf(stderr, "InternetOpen failed\n");
        return;
    }

    // Open a connection to a server (replace the URL with the desired endpoint)
    hConnect = InternetOpenUrlA(hInternet, "http://www.example.com", NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hConnect == NULL) {
        fprintf(stderr, "InternetOpenUrlA failed\n");
        InternetCloseHandle(hInternet);
        return;
    }

    // Read the response headers
    if (HttpQueryInfoA(hConnect, HTTP_QUERY_DATE, buffer, &bufferSize, NULL)) {
        printf("Date in HTTP Response: %s\n", buffer);
    } else {
        fprintf(stderr, "HttpQueryInfoA failed\n");
    }

    // Clean up
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
}

int main() {
    getDateFromHttpResponse();

    return 0;
}
