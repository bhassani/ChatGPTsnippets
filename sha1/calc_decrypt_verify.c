#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#define SHA1_LENGTH 20
#define ENCRYPTED_HASH_LENGTH 80  // 80 bytes for the encrypted SHA1

void handle_openssl_error() {
    ERR_print_errors_fp(stderr);
    abort();
}

void sha1_hash(const unsigned char *data, size_t len, unsigned char *hash) {
    SHA_CTX sha_ctx;
    SHA1_Init(&sha_ctx);
    SHA1_Update(&sha_ctx, data, len);
    SHA1_Final(hash, &sha_ctx);
}

int rsa_decrypt(const unsigned char *encrypted, size_t encrypted_len, unsigned char *decrypted, const char *public_key_file) {
    FILE *keyfile = fopen(public_key_file, "rb");
    if (!keyfile) {
        perror("Unable to open public key file");
        return -1;
    }

    RSA *rsa = PEM_read_RSA_PUBKEY(keyfile, NULL, NULL, NULL);
    fclose(keyfile);

    if (!rsa) {
        handle_openssl_error();
    }

    int result = RSA_public_decrypt(encrypted_len, encrypted, decrypted, rsa, RSA_PKCS1_PADDING);

    RSA_free(rsa);

    if (result == -1) {
        handle_openssl_error();
    }

    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <binary file> <public key file>\n", argv[0]);
        return 1;
    }

    const char *binary_file = argv[1];
    const char *public_key_file = argv[2];

    FILE *file = fopen(binary_file, "rb");
    if (!file) {
        perror("Unable to open file");
        return 1;
    }

    // Read the first 80 bytes (the encrypted SHA1 hash)
    unsigned char encrypted_hash[ENCRYPTED_HASH_LENGTH];
    fread(encrypted_hash, 1, ENCRYPTED_HASH_LENGTH, file);

    // Get the file size excluding the first 80 bytes
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file) - ENCRYPTED_HASH_LENGTH;
    fseek(file, ENCRYPTED_HASH_LENGTH, SEEK_SET);

    // Read the rest of the file
    unsigned char *file_data = malloc(file_size);
    if (!file_data) {
        perror("Memory allocation error");
        fclose(file);
        return 1;
    }
    fread(file_data, 1, file_size, file);

    // Calculate the SHA1 hash of the file (excluding the first 80 bytes)
    unsigned char sha1_hash_value[SHA1_LENGTH];
    sha1_hash(file_data, file_size, sha1_hash_value);

    // Decrypt the SHA1 hash using the public key
    unsigned char decrypted_hash[SHA1_LENGTH];
    if (rsa_decrypt(encrypted_hash, ENCRYPTED_HASH_LENGTH, decrypted_hash, public_key_file) == -1) {
        free(file_data);
        fclose(file);
        return 1;
    }

    // Compare the decrypted SHA1 hash with the calculated SHA1 hash
    if (memcmp(decrypted_hash, sha1_hash_value, SHA1_LENGTH) == 0) {
        printf("SHA1 hash matches!\n");
    } else {
        printf("SHA1 hash does not match!\n");
    }

    free(file_data);
    fclose(file);

    return 0;
}
