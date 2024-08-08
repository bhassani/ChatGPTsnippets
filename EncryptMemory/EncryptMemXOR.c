#include <stdio.h>
#include <windows.h>

void encrypt(char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        data[i] ^= 0xAA; // Simple XOR encryption
    }
}

void decrypt(char *data, size_t len) {
    // XOR encryption is symmetric, so decryption is the same as encryption
    encrypt(data, len);
}

char *protectedMemory;
size_t protectedSize;

LONG WINAPI MyExceptionHandler(EXCEPTION_POINTERS *ExceptionInfo) {
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        DWORD oldProtect;

        // Check if the faulting address is within our protected memory region
        if ((BYTE*)ExceptionInfo->ExceptionRecord->ExceptionAddress >= (BYTE*)protectedMemory &&
            (BYTE*)ExceptionInfo->ExceptionRecord->ExceptionAddress < (BYTE*)protectedMemory + protectedSize) {

            // Decrypt memory before allowing access
            VirtualProtect(protectedMemory, protectedSize, PAGE_READWRITE, &oldProtect);
            decrypt(protectedMemory, protectedSize);

            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void ProtectMemory(char *memory, size_t size) {
    DWORD oldProtect;

    // Encrypt the memory
    encrypt(memory, size);

    // Protect the memory so any access triggers an exception
    VirtualProtect(memory, size, PAGE_NOACCESS, &oldProtect);
}

int main() {
    // Set up the exception handler
    AddVectoredExceptionHandler(1, MyExceptionHandler);

    // Allocate memory and copy some data into it
    protectedSize = 128;
    protectedMemory = (char*)VirtualAlloc(NULL, protectedSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!protectedMemory) {
        printf("Failed to allocate memory\n");
        return 1;
    }

    strcpy(protectedMemory, "Sensitive Data");

    // Protect the memory (encrypt it and make it non-accessible)
    ProtectMemory(protectedMemory, protectedSize);

    // Simulate accessing the protected memory
    printf("Accessing protected memory...\n");

    // This line will trigger the exception handler to decrypt the memory
    printf("Protected data: %s\n", protectedMemory);

    // Re-protect the memory after use
    ProtectMemory(protectedMemory, protectedSize);

    // Cleanup
    VirtualFree(protectedMemory, 0, MEM_RELEASE);
    RemoveVectoredExceptionHandler(MyExceptionHandler);

    return 0;
}
