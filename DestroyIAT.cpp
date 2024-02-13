#include <windows.h>
#include <stdio.h>

// Function pointer typedef for the function we'll destroy in the IAT
typedef void (*FunctionToDestroy)();

// Function to "destroy" the IAT entry for a given function
void DestroyIATEntry(const char* moduleName, const char* functionName) {
    // Load the DLL containing the function
    HMODULE hModule = GetModuleHandleA(moduleName);
    if (hModule == NULL) {
        printf("Failed to load module: %s\n", moduleName);
        return;
    }

    // Get the address of the function in the IAT
    FunctionToDestroy* pFunction = (FunctionToDestroy*)GetProcAddress(hModule, functionName);
    if (pFunction == NULL) {
        printf("Failed to get address of function: %s\n", functionName);
        return;
    }

    // Overwrite the IAT entry with NULL
    *pFunction = NULL;

    printf("Destroyed IAT entry for function: %s\n", functionName);
}

int main() {
    // Example: Destroy the IAT entry for "MessageBoxA" function in "user32.dll"
    DestroyIATEntry("user32.dll", "MessageBoxA");

    // Now any attempt to call MessageBoxA would result in an error

    return 0;
}
