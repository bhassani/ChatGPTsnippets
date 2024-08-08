#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <wincrypt.h>

#pragma comment (lib, "advapi32.lib")

#define RSA_KEYSIZE 4096
#define SHA1_HASH_SIZE 20

void handle_error(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

void hash_file(const char *filename, BYTE *hash) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HANDLE hFile;
    BYTE buffer[1024];
    DWORD bytesRead;
    DWORD hashLen = SHA1_HASH_SIZE;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        handle_error("CryptAcquireContext failed");
    }

    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
        handle_error("CryptCreateHash failed");
    }

    hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        handle_error("CreateFile failed");
    }

    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        if (!CryptHashData(hHash, buffer, bytesRead, 0)) {
            handle_error("CryptHashData failed");
        }
    }

    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        handle_error("CryptGetHashParam failed");
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);
}

void rc4_encrypt_decrypt(const BYTE *key, BYTE *data, DWORD dataLen) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        handle_error("CryptAcquireContext failed");
    }

    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
        handle_error("CryptCreateHash failed");
    }

    if (!CryptHashData(hHash, key, SHA1_HASH_SIZE, 0)) {
        handle_error("CryptHashData failed");
    }

    if (!CryptDeriveKey(hProv, CALG_RC4, hHash, 0, &hKey)) {
        handle_error("CryptDeriveKey failed");
    }

    if (!CryptEncrypt(hKey, 0, TRUE, 0, data, &dataLen, dataLen)) {
        handle_error("CryptEncrypt failed");
    }

    CryptDestroyKey(hKey);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
}

void rsa_sign_hash(const BYTE *hash, BYTE *signature, DWORD *sigLen, const char *privateKeyFilename) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;
    HANDLE hKeyFile = CreateFile(privateKeyFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    BYTE *privateKey = NULL;
    DWORD keyLen;

    if (hKeyFile == INVALID_HANDLE_VALUE) {
        handle_error("CreateFile for private key failed");
    }

    // Load private key
    keyLen = GetFileSize(hKeyFile, NULL);
    privateKey = (BYTE *)malloc(keyLen);
    ReadFile(hKeyFile, privateKey, keyLen, &keyLen, NULL);
    CloseHandle(hKeyFile);

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        handle_error("CryptAcquireContext failed");
    }

    if (!CryptImportKey(hProv, privateKey, keyLen, 0, CRYPT_EXPORTABLE, &hKey)) {
        handle_error("CryptImportKey failed");
    }

    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
        handle_error("CryptCreateHash failed");
    }

    if (!CryptHashData(hHash, hash, SHA1_HASH_SIZE, 0)) {
        handle_error("CryptHashData failed");
    }

    *sigLen = RSA_keysize; // Ensure this is correctly defined or obtained
    if (!CryptSignHash(hHash, AT_KEYEXCHANGE, NULL, 0, signature, sigLen)) {
        handle_error("CryptSignHash failed");
    }

    CryptDestroyKey(hKey);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    free(privateKey);
}

BOOL rsa_verify_hash(const BYTE *hash, const BYTE *signature, DWORD sigLen, const char *publicKeyFilename) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;
    HANDLE hKeyFile = CreateFile(publicKeyFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    BYTE *publicKey = NULL;
    DWORD keyLen;
    BOOL result = FALSE;

    if (hKeyFile == INVALID_HANDLE_VALUE) {
        handle_error("CreateFile for public key failed");
    }

    // Load public key
    keyLen = GetFileSize(hKeyFile, NULL);
    publicKey = (BYTE *)malloc(keyLen);
    ReadFile(hKeyFile, publicKey, keyLen, &keyLen, NULL);
    CloseHandle(hKeyFile);

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        handle_error("CryptAcquireContext failed");
    }

    if (!CryptImportKey(hProv, publicKey, keyLen, 0, CRYPT_VERIFYCONTEXT, &hKey)) {
        handle_error("CryptImportKey failed");
    }

    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
        handle_error("CryptCreateHash failed");
    }

    if (!CryptHashData(hHash, hash, SHA1_HASH_SIZE, 0)) {
        handle_error("CryptHashData failed");
    }

    result = CryptVerifySignature(hHash, signature, sigLen, hKey, NULL, 0);

    CryptDestroyKey(hKey);
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    free(publicKey);

    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file> <private_key> <public_key>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *privateKeyFilename = argv[2];
    const char *publicKeyFilename = argv[3];

    BYTE hash[SHA1_HASH_SIZE];
    hash_file(filename, hash);

    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        handle_error("CreateFile failed");
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    BYTE *fileData = (BYTE *)malloc(fileSize);
    ReadFile(hFile, fileData, fileSize, &fileSize, NULL);
    CloseHandle(hFile);

    rc4_encrypt_decrypt(hash, fileData, fileSize);

    BYTE signature[RSA_keysize];
    DWORD sigLen;
    rsa_sign_hash(hash, signature, &sigLen, privateKeyFilename);

    if (rsa_verify_hash(hash, signature, sigLen, publicKeyFilename)) {
        printf("Signature is valid.\n");

        rc4_encrypt_decrypt(hash, fileData, fileSize);

        // Execute the file
        HANDLE hTempFile = CreateFile("temp_exec_file", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTempFile == INVALID_HANDLE_VALUE) {
            handle_error("CreateFile for temp_exec_file failed");
        }
        DWORD written;
        WriteFile(hTempFile, fileData, fileSize, &written, NULL);
        CloseHandle(hTempFile);

        //use createprocess but ok
        system("temp_exec_file");

        //DeleteFile("temp_exec_file");
    } else {
        fprintf(stderr, "Invalid signature.\n");
    }

    free(fileData);
    return 0;
}
