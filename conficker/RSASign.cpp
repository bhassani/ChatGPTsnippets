/*
The sign_binary function takes the binary file, private key file, and signature file paths as arguments. It reads the binary file, generates a SHA-256 hash of its contents, signs the hash with the private key, and writes the resulting signature to the signature file.
The verify_signature function takes the binary file, signature file, and public key file paths as arguments. It reads the binary file, generates a SHA-256 hash of its contents, reads the signature from the signature file, and verifies the signature using the public key.
Please make sure to replace "binary.exe", "private_key.pem", "signature.bin", and "public_key.pem" with the appropriate file paths for your files.

You'll need to link against the OpenSSL library when compiling. Additionally, ensure you include the appropriate header files and handle errors properly in a real-world scenario
*/

#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

#define SIGNATURE_SIZE 256

int sign_binary(const char* binary_file, const char* private_key_file, const char* signature_file) {
    FILE* f_bin = fopen(binary_file, "rb");
    if (!f_bin) {
        printf("Failed to open binary file\n");
        return -1;
    }

    FILE* f_priv_key = fopen(private_key_file, "r");
    if (!f_priv_key) {
        printf("Failed to open private key file\n");
        fclose(f_bin);
        return -1;
    }

    RSA* rsa = PEM_read_RSAPrivateKey(f_priv_key, NULL, NULL, NULL);
    if (!rsa) {
        printf("Failed to read private key\n");
        fclose(f_bin);
        fclose(f_priv_key);
        return -1;
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned int hash_len;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f_bin)) > 0) {
        SHA256_Update(&sha256, buffer, bytes_read);
    }

    SHA256_Final(hash, &sha256);

    unsigned char signature[SIGNATURE_SIZE];
    unsigned int signature_len;

    if (!RSA_sign(NID_sha256, hash, sizeof(hash), signature, &signature_len, rsa)) {
        printf("Failed to sign binary\n");
        RSA_free(rsa);
        fclose(f_bin);
        fclose(f_priv_key);
        return -1;
    }

    FILE* f_signature = fopen(signature_file, "wb");
    if (!f_signature) {
        printf("Failed to open signature file\n");
        RSA_free(rsa);
        fclose(f_bin);
        fclose(f_priv_key);
        return -1;
    }

    fwrite(signature, 1, signature_len, f_signature);

    RSA_free(rsa);
    fclose(f_bin);
    fclose(f_priv_key);
    fclose(f_signature);

    return 0;
}

int verify_signature(const char* binary_file, const char* signature_file, const char* public_key_file) {
    FILE* f_bin = fopen(binary_file, "rb");
    if (!f_bin) {
        printf("Failed to open binary file\n");
        return -1;
    }

    FILE* f_signature = fopen(signature_file, "rb");
    if (!f_signature) {
        printf("Failed to open signature file\n");
        fclose(f_bin);
        return -1;
    }

    FILE* f_pub_key = fopen(public_key_file, "r");
    if (!f_pub_key) {
        printf("Failed to open public key file\n");
        fclose(f_bin);
        fclose(f_signature);
        return -1;
    }

    RSA* rsa = PEM_read_RSA_PUBKEY(f_pub_key, NULL, NULL, NULL);
    if (!rsa) {
        printf("Failed to read public key\n");
        fclose(f_bin);
        fclose(f_signature);
        fclose(f_pub_key);
        return -1;
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned int hash_len;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f_bin)) > 0) {
        SHA256_Update(&sha256, buffer, bytes_read);
    }

    SHA256_Final(hash, &sha256);

    unsigned char signature[SIGNATURE_SIZE];
    unsigned int signature_len = fread(signature, 1, SIGNATURE_SIZE, f_signature);

    int ret = RSA_verify(NID_sha256, hash, sizeof(hash), signature, signature_len, rsa);
    if (ret == 1) {
        printf("Signature verified successfully\n");
    } else if (ret == 0) {
        printf("Signature verification failed\n");
    } else {
        printf("Error verifying signature\n");
    }

    RSA_free(rsa);
    fclose(f_bin);
    fclose(f_signature);
    fclose(f_pub_key);

    return ret;
}

int main() {
    const char* binary_file = "binary.exe";
    const char* private_key_file = "private_key.pem";
    const char* signature_file = "signature.bin";
    const char* public_key_file = "public_key.pem";

    // Sign the binary
    if (sign_binary(binary_file, private_key_file, signature_file) != 0) {
        printf("Failed to sign binary\n");
        return -1;
    }

    // Verify the signature
    if (verify_signature(binary_file, signature_file, public_key_file) != 0) {
        printf("Signature verification failed\n");
        return -1;
    }

    return 0;
}
