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

int rsa_encrypt(const unsigned char *data, size_t data_len, unsigned char *encrypted, const char *private_key_file) {
    FILE *keyfile = fopen(private_key_file, "rb");
    if (!keyfile) {
        perror("Unable to open private key file");
        return -1;
    }

    RSA *rsa = PEM_read_RSAPrivateKey(keyfile, NULL, NULL, NULL);
    fclose(keyfile);

    if (!rsa) {
        handle_openssl_error();
    }

    int result = RSA_private_encrypt(data_len, data, encrypted, rsa, RSA_PKCS1_PADDING);

    RSA_free(rsa);

    if (result == -1) {
        handle_openssl_error();
    }

    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <binary file> <private key file>\n", argv[0]);
        return 1;
    }

    const char *binary_file = argv[1];
    const char *private_key_file = argv[2];

    FILE *file = fopen(binary_file, "rb+");
    if (!file) {
        perror("Unable to open file");
        return 1;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read the entire file
    unsigned char *file_data = malloc(file_size);
    if (!file_data) {
        perror("Memory allocation error");
        fclose(file);
        return 1;
    }
    fread(file_data, 1, file_size, file);

    // Calculate the SHA1 hash of the file
    unsigned char sha1_hash_value[SHA1_LENGTH];
    sha1_hash(file_data, file_size, sha1_hash_value);

    // Encrypt the SHA1 hash using the private key
    unsigned char encrypted_hash[ENCRYPTED_HASH_LENGTH];
    if (rsa_encrypt(sha1_hash_value, SHA1_LENGTH, encrypted_hash, private_key_file) == -1) {
        free(file_data);
        fclose(file);
        return 1;
    }

    // Prepend the encrypted hash to the file
    FILE *outfile = fopen(binary_file, "wb");
    if (!outfile) {
        perror("Unable to open file for writing");
        free(file_data);
        fclose(file);
        return 1;
    }

    fwrite(encrypted_hash, 1, ENCRYPTED_HASH_LENGTH, outfile);
    fwrite(file_data, 1, file_size, outfile);

    free(file_data);
    fclose(outfile);

    return 0;
}
