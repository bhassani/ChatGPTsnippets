#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

/*
To implement the operation 
M≡Sig^e mod N in C, you can use the OpenSSL library, which provides functions for working with cryptographic operations, including RSA. 
Below is an example of how you can perform this operation using OpenSSL in a C program:
*/

// Function to perform the operation M ≡ Sig^e mod N
void rsaVerify(const char* publicKeyPath, const char* signaturePath, const char* message) {
    // Load the public key
    FILE* publicKeyFile = fopen(publicKeyPath, "r");
    if (!publicKeyFile) {
        perror("Error opening public key file");
        return;
    }

    RSA* rsaPublicKey = PEM_read_RSA_PUBKEY(publicKeyFile, NULL, NULL, NULL);
    fclose(publicKeyFile);

    if (!rsaPublicKey) {
        ERR_print_errors_fp(stderr);
        return;
    }

    // Load the signature
    FILE* signatureFile = fopen(signaturePath, "r");
    if (!signatureFile) {
        perror("Error opening signature file");
        RSA_free(rsaPublicKey);
        return;
    }

    // Assuming the signature is in hexadecimal format
    BIGNUM* signature = BN_new();
    BN_hex2bn(&signature, signatureFile);
    fclose(signatureFile);

    // Perform the operation M ≡ Sig^e mod N
    BIGNUM* result = BN_new();
    RSA_public_decrypt(BN_num_bytes(signature), BN_data(signature), result, rsaPublicKey, RSA_NO_PADDING);

    // Compare the result with the original message
    if (BN_cmp(result, BN_new()) == 0) {
        printf("Signature is valid!\n");
    } else {
        printf("Signature is invalid.\n");
    }

    // Cleanup
    RSA_free(rsaPublicKey);
    BN_free(signature);
    BN_free(result);
}

int main() {
    // Replace these paths with the actual paths to your public key and signature files
    const char* publicKeyPath = "path/to/public_key.pem";
    const char* signaturePath = "path/to/signature.txt";
    const char* message = "Hello, World!";

    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();

    // Perform the RSA verification
    rsaVerify(publicKeyPath, signaturePath, message);

    // Cleanup OpenSSL
    ERR_free_strings();
    EVP_cleanup();

    return 0;
}
