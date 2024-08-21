#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>

#pragma comment(lib, "advapi32.lib")

#define AES_KEY_SIZE 16  // AES-128
#define AES_BLOCK_SIZE 16

// AES key (for demonstration, should be securely generated in a real-world scenario)
BYTE aesKey[AES_KEY_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10 };

void EncryptDecryptMemory(BYTE *buffer, SIZE_T size, BOOL encrypt) {
    HCRYPTPROV hProv;
    HCRYPTKEY hKey;
    HCRYPTHASH hHash;

    // Acquire a cryptographic provider context handle
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        printf("CryptAcquireContext failed. Error: %d\n", GetLastError());
        return;
    }

    // Create a hash object
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        printf("CryptCreateHash failed. Error: %d\n", GetLastError());
        CryptReleaseContext(hProv, 0);
        return;
    }

    // Hash the AES key
    if (!CryptHashData(hHash, aesKey, AES_KEY_SIZE, 0)) {
        printf("CryptHashData failed. Error: %d\n", GetLastError());
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return;
    }

    // Generate the AES key
    if (!CryptDeriveKey(hProv, CALG_AES_128, hHash, 0, &hKey)) {
        printf("CryptDeriveKey failed. Error: %d\n", GetLastError());
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return;
    }

    // Perform encryption/decryption
    DWORD dwSize = (DWORD)size;
    if (encrypt) {
        if (!CryptEncrypt(hKey, 0, TRUE, 0, buffer, &dwSize, dwSize)) {
            printf("CryptEncrypt failed. Error: %d\n", GetLastError());
        }
    } else {
        if (!CryptDecrypt(hKey, 0, TRUE, 0, buffer, &dwSize)) {
            printf("CryptDecrypt failed. Error: %d\n", GetLastError());
        }
    }

    // Clean up
    CryptDestroyKey(hKey);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
}

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
            EncryptDecryptMemory(pSection, pSectionHeader->Misc.VirtualSize, FALSE);
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
            EncryptDecryptMemory(pSection, pSectionHeader->Misc.VirtualSize, TRUE);
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
