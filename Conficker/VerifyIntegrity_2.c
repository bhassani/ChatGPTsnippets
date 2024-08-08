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

BOOL LoadPrivateKeyFromFile(const char* filename, HCRYPTPROV* hProv, HCRYPTKEY* hKey) {
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

    if (!CryptAcquireContext(hProv, NULL, NULL, PROV_RSA_AES, CRYPT_NEWKEYSET)) {
        if (!CryptAcquireContext(hProv, NULL, NULL, PROV_RSA_AES, 0)) {
            PrintError("CryptAcquireContext");
            free(pbKeyBlob);
            return FALSE;
        }
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

BOOL SignHash(HCRYPTHASH hHash, HCRYPTKEY hKey, BYTE** ppbSignature, DWORD* pdwSigLen) {
    *pdwSigLen = 0;
    if (!CryptSignHash(hHash, AT_SIGNATURE, NULL, 0, NULL, pdwSigLen)) {
        PrintError("CryptSignHash (get size)");
        return FALSE;
    }

    *ppbSignature = (BYTE*)malloc(*pdwSigLen);
    if (!CryptSignHash(hHash, AT_SIGNATURE, NULL, 0, *ppbSignature, pdwSigLen)) {
        PrintError("CryptSignHash");
        free(*ppbSignature);
        return FALSE;
    }

    return TRUE;
}

BOOL SaveSignatureToFile(const char* filename, const BYTE* pbSignature, DWORD dwSigLen) {
    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        PrintError("CreateFile");
        return FALSE;
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, pbSignature, dwSigLen, &bytesWritten, NULL) || bytesWritten != dwSigLen) {
        PrintError("WriteFile");
        CloseHandle(hFile);
        return FALSE;
    }

    CloseHandle(hFile);
    return TRUE;
}

int main() {
    const char* binaryFilename = "path_to_your_binary_file";
    const char* privKeyFilename = "path_to_your_private_key_file";
    const char* signatureFilename = "path_to_save_signature";

    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;
    HCRYPTHASH hHash = 0;
    BYTE* pbSignature = NULL;
    DWORD dwSigLen = 0;
    BOOL result = FALSE;

    if (!LoadPrivateKeyFromFile(privKeyFilename, &hProv, &hKey)) {
        goto cleanup;
    }

    if (!HashFile(binaryFilename, hProv, &hHash)) {
        goto cleanup;
    }

    if (!SignHash(hHash, hKey, &pbSignature, &dwSigLen)) {
        goto cleanup;
    }

    if (!SaveSignatureToFile(signatureFilename, pbSignature, dwSigLen)) {
        goto cleanup;
    }

    printf("Signature saved to %s\n", signatureFilename);
    result = TRUE;

cleanup:
    if (pbSignature) free(pbSignature);
    if (hHash) CryptDestroyHash(hHash);
    if (hKey) CryptDestroyKey(hKey);
    if (hProv) CryptReleaseContext(hProv, 0);
    return result ? 0 : 1;
}
