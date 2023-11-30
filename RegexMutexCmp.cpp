#include <windows.h>
#include <stdio.h>
#include <regex.h>

#define MAX_MUTEX_NAME_LENGTH 256

// Function to check if a mutex matches the specified regular expression
BOOL IsMutexMatch(const char* mutexName, const char* regexPattern) {
    regex_t regex;

    if (regcomp(&regex, regexPattern, REG_EXTENDED | REG_NOSUB) != 0) {
        return FALSE; // Failed to compile regular expression
    }

    int match = regexec(&regex, mutexName, 0, NULL, 0);
    regfree(&regex);

    return match == 0; // Returns TRUE if there is a match
}

// Function to enumerate existing mutexes and check for a match
void EnumerateMutexes(const char* regexPattern) {
    HANDLE hMutexSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

    if (hMutexSnap == INVALID_HANDLE_VALUE) {
        printf("Error creating mutex snapshot.\n");
        return;
    }

    MUTEXENTRY32 me32;
    me32.dwSize = sizeof(MUTEXENTRY32);

    if (!Mutex32First(hMutexSnap, &me32)) {
        printf("Error enumerating mutexes.\n");
        CloseHandle(hMutexSnap);
        return;
    }

    do {
        if (IsMutexMatch(me32.szMutexName, regexPattern)) {
            printf("Match found: %s\n", me32.szMutexName);
            // You may want to take specific actions if a matching mutex is found
        }
    } while (Mutex32Next(hMutexSnap, &me32));

    CloseHandle(hMutexSnap);
}

int main() {
    const char* regexPattern = "^DarkSideMutex_\\w{8}$";
    
    EnumerateMutexes(regexPattern);

    return 0;
}
