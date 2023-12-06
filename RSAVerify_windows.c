#include <windows.h>
#include <wincrypt.h>
#include <tchar.h>

// Function to perform the operation M ≡ Sig^e mod N using CryptoAPI
BOOL rsaVerify(const BYTE* signature, DWORD signatureSize, const BYTE* publicKey, DWORD publicKeySize, const BYTE* message, DWORD messageSize) {
    HCRYPTPROV hCryptProv = 0;
    HCRYPTKEY hPublicKey = 0;

    // Acquire a cryptographic provider context
    if (!CryptAcquireContext(&hCryptProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        _tprintf(_T("Error during CryptAcquireContext (error %x)\n"), GetLastError());
        return FALSE;
    }

    // Import the public key
    if (!CryptImportKey(hCryptProv, publicKey, publicKeySize, 0, 0, &hPublicKey)) {
        _tprintf(_T("Error during CryptImportKey (error %x)\n"), GetLastError());
        CryptReleaseContext(hCryptProv, 0);
        return FALSE;
    }

    // Perform the RSA operation
    if (!CryptVerifySignature(hPublicKey, signature, signatureSize, 0, NULL, 0)) {
        _tprintf(_T("Error during CryptVerifySignature (error %x)\n"), GetLastError());
        CryptDestroyKey(hPublicKey);
        CryptReleaseContext(hCryptProv, 0);
        return FALSE;
    }

    _tprintf(_T("Signature is valid!\n"));

    // Cleanup
    CryptDestroyKey(hPublicKey);
    CryptReleaseContext(hCryptProv, 0);
    return TRUE;
}

int main() {
    // Replace these values with the actual public key, signature, and message
    const BYTE publicKey[] = { /* Your public key bytes here */ };
    const DWORD publicKeySize = sizeof(publicKey);

    const BYTE signature[] = { /* Your signature bytes here */ };
    const DWORD signatureSize = sizeof(signature);

    const BYTE message[] = { /* Your original message bytes here */ };
    const DWORD messageSize = sizeof(message);

    // Perform the RSA verification
    if (!rsaVerify(signature, signatureSize, publicKey, publicKeySize, message, messageSize)) {
        _tprintf(_T("Signature is invalid.\n"));
        return 1;
    }

    return 0;
}
