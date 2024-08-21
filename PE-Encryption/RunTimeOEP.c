#include <windows.h>
#include <stdio.h>

#define XOR_KEY 0xAA  // XOR key for encryption/decryption

// Function to encrypt/decrypt memory using XOR
void EncryptDecryptMemory(BYTE *buffer, SIZE_T size) {
    for (SIZE_T i = 0; i < size; i++) {
        buffer[i] ^= XOR_KEY;
    }
}

// Function to find the entry point and decrypt the .text section
DWORD WINAPI DecryptAndExecute(LPVOID param) {
    LPVOID baseAddress = GetModuleHandle(NULL);

    // Retrieve the DOS header
    PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)baseAddress;
    if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("Invalid DOS signature.\n");
        return 1;
    }

    // Retrieve the NT headers
    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((BYTE *)baseAddress + pDOSHeader->e_lfanew);
    if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE) {
        printf("Invalid NT signature.\n");
        return 1;
    }

    // Locate the .text section (code section)
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNTHeaders);
    for (int i = 0; i < pNTHeaders->FileHeader.NumberOfSections; i++) {
        if (strcmp((char *)pSectionHeader->Name, ".text") == 0) {
            // Decrypt the .text section
            BYTE *pSection = (BYTE *)baseAddress + pSectionHeader->VirtualAddress;
            EncryptDecryptMemory(pSection, pSectionHeader->Misc.VirtualSize);
            printf(".text section decrypted.\n");
            break;
        }
        pSectionHeader++;
    }

    // Get the Original Entry Point (OEP)
    DWORD oepRVA = pNTHeaders->OptionalHeader.AddressOfEntryPoint;
    LPVOID oepAddress = (BYTE *)baseAddress + oepRVA;

    printf("Jumping to Original Entry Point (OEP)...\n");

    // Cast the OEP address to a function pointer and jump to it
    __asm {
        mov eax, oepAddress
        jmp eax
    }

    return 0;
}

int main() {
    LPVOID baseAddress = GetModuleHandle(NULL);

    // Retrieve the DOS header
    PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)baseAddress;
    if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("Invalid DOS signature.\n");
        return 1;
    }

    // Retrieve the NT headers
    PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((BYTE *)baseAddress + pDOSHeader->e_lfanew);
    if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE) {
        printf("Invalid NT signature.\n");
        return 1;
    }

    // Locate the .text section (code section)
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNTHeaders);
    for (int i = 0; i < pNTHeaders->FileHeader.NumberOfSections; i++) {
        if (strcmp((char *)pSectionHeader->Name, ".text") == 0) {
            // Encrypt the .text section
            BYTE *pSection = (BYTE *)baseAddress + pSectionHeader->VirtualAddress;
            EncryptDecryptMemory(pSection, pSectionHeader->Misc.VirtualSize);
            printf(".text section encrypted.\n");
            break;
        }
        pSectionHeader++;
    }

    // Create a new thread to decrypt and execute the original code
    HANDLE hThread = CreateThread(NULL, 0, DecryptAndExecute, NULL, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    return 0;
}
