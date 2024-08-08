#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>

#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

void PrintError(const char* msg) {
    DWORD err = GetLastError();
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        err,
        0,
        (LPTSTR) &lpMsgBuf,
        0,
        NULL);
    printf("%s failed with error %lu: %s\n", msg, err, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

BOOL LoadPublicKeyFromFile(const char* filename, HCRYPTPROV* hProv, HCRYPTKEY* hKey) {
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        PrintError("CreateFile");
        return FALSE;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    BYTE* pbKeyBlob = (BYTE*)malloc(fileSize);

    DWORD bytesRead;
    if (!ReadFile(hFile, pbKeyBlob, fileSize, &bytesRead, NULL) || bytesRead != fileSize) {
        PrintError("ReadFile");
        free(pbKeyBlob);
        CloseHandle(hFile);
        return FALSE;
    }
    CloseHandle(hFile);

    if (!CryptAcquireContext(hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        PrintError("CryptAcquireContext");
        free(pbKeyBlob);
        return FALSE;
    }

    if (!CryptImportKey(*hProv, pbKeyBlob, fileSize, 0, 0, hKey)) {
        PrintError("CryptImportKey");
        free(pbKeyBlob);
        CryptReleaseContext(*hProv, 0);
        return FALSE;
    }

    free(pbKeyBlob);
    return TRUE;
}

BOOL HashFile(const char* filename, HCRYPTPROV hProv, HCRYPTHASH* hHash) {
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        PrintError("CreateFile");
        return FALSE;
    }

    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, hHash)) {
        PrintError("CryptCreateHash");
        CloseHandle(hFile);
        return FALSE;
    }

    BYTE buffer[1024];
    DWORD bytesRead;
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        if (!CryptHashData(*hHash, buffer, bytesRead, 0)) {
            PrintError("CryptHashData");
            CryptDestroyHash(*hHash);
            CloseHandle(hFile);
            return FALSE;
        }
    }

    CloseHandle(hFile);
    return TRUE;
}

BOOL VerifySignature(const char* binaryFilename, const char* pubKeyFilename, const BYTE* signature, DWORD sigLen) {
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;
    HCRYPTHASH hHash = 0;
    BOOL result = FALSE;

    if (!LoadPublicKeyFromFile(pubKeyFilename, &hProv, &hKey)) {
        goto cleanup;
    }

    if (!HashFile(binaryFilename, hProv, &hHash)) {
        goto cleanup;
    }

    result = CryptVerifySignature(hHash, signature, sigLen, hKey, NULL, 0);
    if (!result) {
        PrintError("CryptVerifySignature");
    }

cleanup:
    if (hHash) CryptDestroyHash(hHash);
    if (hKey) CryptDestroyKey(hKey);
    if (hProv) CryptReleaseContext(hProv, 0);
    return result;
}

int main() {
    const char* binaryFilename = "path_to_your_binary_file";
    const char* pubKeyFilename = "path_to_your_public_key_file";
    const BYTE signature[] = { /* signature bytes here */ };
    DWORD sigLen = sizeof(signature);

    if (VerifySignature(binaryFilename, pubKeyFilename, signature, sigLen)) {
        printf("Signature is valid.\n");
    } else {
        printf("Signature is not valid.\n");
    }

    return 0;
}
