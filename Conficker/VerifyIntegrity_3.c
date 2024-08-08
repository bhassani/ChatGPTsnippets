#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>

#pragma comment(lib, "crypt32.lib")

#define SIGNATURE_SIZE 256 // Assuming RSA 2048

void print_error(const char *message, DWORD err) {
    fprintf(stderr, "%s. Error code: %lu\n", message, err);
}

int main() {
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;
    HCRYPTHASH hHash = 0;
    BYTE pbSignature[SIGNATURE_SIZE];
    DWORD dwSigLen = SIGNATURE_SIZE;
    BYTE *pbBuffer = NULL;
    DWORD dwBufferLen;
    BOOL result;

    // Load public key from file
    FILE *keyFile = fopen("public_key.cer", "rb");
    if (!keyFile) {
        print_error("Error opening public key file", GetLastError());
        return -1;
    }
    fseek(keyFile, 0, SEEK_END);
    DWORD dwKeyBlobSize = ftell(keyFile);
    fseek(keyFile, 0, SEEK_SET);
    BYTE *pbKeyBlob = (BYTE*)malloc(dwKeyBlobSize);
    fread(pbKeyBlob, 1, dwKeyBlobSize, keyFile);
    fclose(keyFile);

    CRYPT_DATA_BLOB keyBlob;
    keyBlob.pbData = pbKeyBlob;
    keyBlob.cbData = dwKeyBlobSize;

    PCCERT_CONTEXT pCertContext = NULL;
    pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, pbKeyBlob, dwKeyBlobSize);
    if (!pCertContext) {
        print_error("CertCreateCertificateContext failed", GetLastError());
        free(pbKeyBlob);
        return -1;
    }

    // Acquire context
    result = CryptAcquireContext(&hProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, 0);
    if (!result) {
        print_error("CryptAcquireContext failed", GetLastError());
        CertFreeCertificateContext(pCertContext);
        free(pbKeyBlob);
        return -1;
    }

    // Import the public key
    result = CryptImportPublicKeyInfo(hProv, X509_ASN_ENCODING, &pCertContext->pCertInfo->SubjectPublicKeyInfo, &hKey);
    if (!result) {
        print_error("CryptImportPublicKeyInfo failed", GetLastError());
        CertFreeCertificateContext(pCertContext);
        CryptReleaseContext(hProv, 0);
        free(pbKeyBlob);
        return -1;
    }

    // Read binary content to be verified
    FILE *binFile = fopen("binary_data.bin", "rb");
    if (!binFile) {
        print_error("Error opening binary data file", GetLastError());
        CertFreeCertificateContext(pCertContext);
        CryptReleaseContext(hProv, 0);
        free(pbKeyBlob);
        return -1;
    }
    fseek(binFile, 0, SEEK_END);
    dwBufferLen = ftell(binFile);
    fseek(binFile, 0, SEEK_SET);
    pbBuffer = (BYTE*)malloc(dwBufferLen);
    fread(pbBuffer, 1, dwBufferLen, binFile);
    fclose(binFile);

    // Create hash
    result = CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash);
    if (!result) {
        print_error("CryptCreateHash failed", GetLastError());
        CertFreeCertificateContext(pCertContext);
        CryptReleaseContext(hProv, 0);
        free(pbKeyBlob);
        free(pbBuffer);
        return -1;
    }

    // Hash the binary data
    result = CryptHashData(hHash, pbBuffer, dwBufferLen, 0);
    if (!result) {
        print_error("CryptHashData failed", GetLastError());
        CryptDestroyHash(hHash);
        CertFreeCertificateContext(pCertContext);
        CryptReleaseContext(hProv, 0);
        free(pbKeyBlob);
        free(pbBuffer);
        return -1;
    }

    // Load signature from file
    FILE *sigFile = fopen("signature.bin", "rb");
    if (!sigFile) {
        print_error("Error opening signature file", GetLastError());
        CryptDestroyHash(hHash);
        CertFreeCertificateContext(pCertContext);
        CryptReleaseContext(hProv, 0);
        free(pbKeyBlob);
        free(pbBuffer);
        return -1;
    }
    fread(pbSignature, 1, dwSigLen, sigFile);
    fclose(sigFile);

    // Verify the signature
    result = CryptVerifySignature(hHash, pbSignature, dwSigLen, hKey, NULL, 0);
    if (result) {
        printf("Signature verification succeeded\n");
    } else {
        print_error("Signature verification failed", GetLastError());
    }

    // Cleanup
    CryptDestroyHash(hHash);
    CertFreeCertificateContext(pCertContext);
    CryptReleaseContext(hProv, 0);
    free(pbKeyBlob);
    free(pbBuffer);

    return 0;
}
