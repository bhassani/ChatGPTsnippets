/*

We first generate an RSA key pair using RSA_generate_key().
We read the binary file into memory.
We generate a SHA-256 hash of the binary data.
We sign the hash using RSA private key with RSA_sign().
The resulting signature is then written to a file.
To compile the code, you'll need to link against the OpenSSL library and include the appropriate header files. Ensure you have OpenSSL installed and set up the necessary include and library paths.

Similarly, you can verify the signature using the corresponding public key using the RSA_verify() function. However, for simplicity, I have not included the verification part in this example.
*/

#include <stdio.h>
#include <stdint.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>

#define RSA_KEY_SIZE 2048
#define SIGNATURE_SIZE RSA_KEY_SIZE / 8

// Function to generate RSA key pair
RSA* generate_rsa_keypair() {
    RSA* rsa = RSA_generate_key(RSA_KEY_SIZE, RSA_F4, NULL, NULL);
    if (!rsa) {
        printf("Failed to generate RSA key pair\n");
        return NULL;
    }
    return rsa;
}

// Function to sign binary data using RSA private key
int rsa_sign(const unsigned char* data, size_t data_len, const RSA* rsa, unsigned char* signature) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(data, data_len, hash);

    unsigned int signature_len;
    if (!RSA_sign(NID_sha256, hash, SHA256_DIGEST_LENGTH, signature, &signature_len, rsa)) {
        printf("Failed to sign data\n");
        return -1;
    }

    return 0;
}

int main() {
    const char* binary_file = "binary.exe";
    const char* private_key_file = "private_key.pem";
    const char* signature_file = "signature.bin";

    // Read the binary file
    FILE* f_bin = fopen(binary_file, "rb");
    if (!f_bin) {
        printf("Failed to open binary file\n");
        return -1;
    }
    fseek(f_bin, 0, SEEK_END);
    size_t binary_size = ftell(f_bin);
    fseek(f_bin, 0, SEEK_SET);
    unsigned char* binary_data = malloc(binary_size);
    if (!binary_data) {
        printf("Memory allocation failed\n");
        fclose(f_bin);
        return -1;
    }
    fread(binary_data, 1, binary_size, f_bin);
    fclose(f_bin);

    // Generate RSA key pair
    RSA* rsa = generate_rsa_keypair();
    if (!rsa) {
        printf("Failed to generate RSA key pair\n");
        free(binary_data);
        return -1;
    }

    // Sign the binary data
    unsigned char signature[SIGNATURE_SIZE];
    if (rsa_sign(binary_data, binary_size, rsa, signature) != 0) {
        printf("Failed to sign binary\n");
        RSA_free(rsa);
        free(binary_data);
        return -1;
    }

    // Write the signature to a file
    FILE* f_signature = fopen(signature_file, "wb");
    if (!f_signature) {
        printf("Failed to open signature file\n");
        RSA_free(rsa);
        free(binary_data);
        return -1;
    }
    fwrite(signature, 1, SIGNATURE_SIZE, f_signature);
    fclose(f_signature);

    // Clean up
    RSA_free(rsa);
    free(binary_data);

    printf("Binary signed successfully\n");

    return 0;
}
