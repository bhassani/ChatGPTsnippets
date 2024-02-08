#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LENGTH 100
#define MAX_FILE_COUNT 100
#define ENCRYPTION_KEY "encryption_key"

typedef struct {
    char name[MAX_FILENAME_LENGTH];
    int size;
    char* content;
} File;

typedef struct {
    char name[MAX_FILENAME_LENGTH];
    int file_count;
    File files[MAX_FILE_COUNT];
} Directory;

Directory root_directory;

// Encrypt content using a simple XOR encryption
void encrypt_content(char* content, int size) {
    for (int i = 0; i < size; i++) {
        content[i] ^= ENCRYPTION_KEY[i % strlen(ENCRYPTION_KEY)];
    }
}

// Decrypt content using the same XOR encryption
void decrypt_content(char* content, int size) {
    encrypt_content(content, size); // XOR encryption is symmetric
}

void vfs_init() {
    strcpy(root_directory.name, "root");
    root_directory.file_count = 0;
}

void vfs_create_file(char* filename, char* content, int size) {
    if (root_directory.file_count < MAX_FILE_COUNT) {
        File new_file;
        strcpy(new_file.name, filename);
        new_file.size = size;
        new_file.content = (char*)malloc(size);
        memcpy(new_file.content, content, size);
        encrypt_content(new_file.content, size); // Encrypt content before storing
        root_directory.files[root_directory.file_count++] = new_file;
    } else {
        printf("Cannot create file. Maximum file count reached.\n");
    }
}

void vfs_delete_file(char* filename) {
    for (int i = 0; i < root_directory.file_count; ++i) {
        if (strcmp(root_directory.files[i].name, filename) == 0) {
            free(root_directory.files[i].content);
            for (int j = i; j < root_directory.file_count - 1; ++j) {
                root_directory.files[j] = root_directory.files[j + 1];
            }
            root_directory.file_count--;
            printf("File %s deleted.\n", filename);
            return;
        }
    }
    printf("File %s not found.\n", filename);
}

void vfs_list_files() {
    printf("Files in root directory:\n");
    for (int i = 0; i < root_directory.file_count; ++i) {
        printf("%s\n", root_directory.files[i].name);
    }
}

void vfs_read_file(char* filename) {
    for (int i = 0; i < root_directory.file_count; ++i) {
        if (strcmp(root_directory.files[i].name, filename) == 0) {
            decrypt_content(root_directory.files[i].content, root_directory.files[i].size);
            printf("Content of file %s:\n%s\n", filename, root_directory.files[i].content);
            encrypt_content(root_directory.files[i].content, root_directory.files[i].size); // Encrypt content again after reading
            return;
        }
    }
    printf("File %s not found.\n", filename);
}

int main() {
    vfs_init();

    vfs_create_file("file1.txt", "This is the content of file1.", 30);
    vfs_create_file("file2.txt", "This is the content of file2.", 30);

    vfs_list_files();

    vfs_read_file("file1.txt");
    vfs_read_file("file2.txt");

    vfs_delete_file("file1.txt");

    vfs_list_files();

    return 0;
}
