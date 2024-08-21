#include <windows.h>
#include <stdio.h>

// XOR key for encryption/decryption
#define XOR_KEY 0xAA

void EncryptDecryptMemory(BYTE *buffer, SIZE_T size) {
    for (SIZE_T i = 0; i < size; i++) {
        buffer[i] ^= XOR_KEY;
    }
}

void EncryptSection(LPVOID baseAddress) {
    // Retrieve the DOS header
    PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)baseAddress;
    if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("Invalid DOS signature.\n");
        return;
    }

    // Retrieve the NT headers
    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((BYTE *)baseAddress + pDOSHeader->e_lfanew);
    if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE) {
        printf("Invalid NT signature.\n");
        return;
    }

    // Find the .text section (code section)
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNTHeaders);
    for (int i = 0; i < pNTHeaders->FileHeader.NumberOfSections; i++) {
        if (strcmp((char *)pSectionHeader->Name, ".text") == 0) {
            // Encrypt the .text section
            BYTE *pSection = (BYTE *)baseAddress + pSectionHeader->VirtualAddress;
            EncryptDecryptMemory(pSection, pSectionHeader->Misc.VirtualSize);
            printf(".text section encrypted/decrypted.\n");
            break;
        }
        pSectionHeader++;
    }
}

DWORD WINAPI DecryptAndExecute(LPVOID param) {
    LPVOID baseAddress = GetModuleHandle(NULL);
    EncryptSection(baseAddress);  // Decrypt the .text section before executing
    printf("Decrypted .text section and now executing the main program.\n");

    // Here we would normally jump to the original entry point or continue execution
    // This example is just illustrative and does not perform a real jump to OEP
    return 0;
}

int main() {
    LPVOID baseAddress = GetModuleHandle(NULL);

    // Encrypt the .text section
    EncryptSection(baseAddress);

    // Create a new thread to decrypt and execute the original code
    HANDLE hThread = CreateThread(NULL, 0, DecryptAndExecute, NULL, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    return 0;
}
