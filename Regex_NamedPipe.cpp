#include <windows.h>
#include <tchar.h>
#include <regex.h>
#include <stdio.h>

int main() {
    // Define the regex pattern
    const char* pattern = "MSSE-[0-9]{4}-server";

    // Compile the regex pattern
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Error compiling regex pattern\n");
        return 1;
    }

    // Enumerate named pipes
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(_T("\\\\.\\pipe\\*"), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error enumerating named pipes\n");
        return 1;
    }

    do {
        // Check if the current file name matches the regex pattern
        if (regexec(&regex, findFileData.cFileName, 0, NULL, 0) == 0) {
            printf("Named pipe found: %s\n", findFileData.cFileName);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    // Clean up
    FindClose(hFind);
    regfree(&regex);

    return 0;
}
