/*

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LENGTH 100
#define MAX_FILE_COUNT 100

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
        root_directory.files[root_directory.file_count++] = new_file;
    } else {
        printf("Cannot create file. Maximum file count reached.\n");
    }
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
            printf("Content of file %s:\n%s\n", filename, root_directory.files[i].content);
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
    vfs_read_file("file3.txt");

    return 0;
}
