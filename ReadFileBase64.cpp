#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <wincrypt.h>

#pragma comment(lib, "crypt32.lib")

char* readFileIntoMemory(const char* filename, DWORD* fileSize) {
    HANDLE fileHandle = CreateFileA(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (fileHandle == INVALID_HANDLE_VALUE) {
        printf("Error opening file.\n");
        return NULL;
    }

    // Get the file size
    *fileSize = GetFileSize(fileHandle, NULL);

    // Allocate memory for the file content
    char* fileContent = (char*)malloc(*fileSize);
    if (fileContent == NULL) {
        CloseHandle(fileHandle);
        printf("Memory allocation failed.\n");
        return NULL;
    }

    // Read the file content into memory
    DWORD bytesRead;
    if (!ReadFile(fileHandle, fileContent, *fileSize, &bytesRead, NULL)) {
        free(fileContent);
        CloseHandle(fileHandle);
        printf("Error reading file.\n");
        return NULL;
    }

    CloseHandle(fileHandle);

    return fileContent;
}

char* base64Encode(const char* data, DWORD dataSize, DWORD* base64Size) {
    // Calculate the size needed for the base64-encoded data
    if (!CryptBinaryToStringA((BYTE*)data, dataSize, CRYPT_STRING_BASE64, NULL, base64Size)) {
        printf("Error calculating base64 size.\n");
        return NULL;
    }

    // Allocate memory for the base64-encoded data
    char* base64Data = (char*)malloc(*base64Size);
    if (base64Data == NULL) {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    // Perform the base64 encoding
    if (!CryptBinaryToStringA((BYTE*)data, dataSize, CRYPT_STRING_BASE64, base64Data, base64Size)) {
        free(base64Data);
        printf("Base64 encoding failed.\n");
        return NULL;
    }

    return base64Data;
}

int main() {
    const char* filename = "example.txt";

    DWORD fileSize;
    char* fileContent = readFileIntoMemory(filename, &fileSize);
    if (fileContent == NULL) {
        return 1;
    }

    DWORD base64Size;
    char* base64Data = base64Encode(fileContent, fileSize, &base64Size);
    if (base64Data == NULL) {
        free(fileContent);
        return 1;
    }

    // Now 'base64Data' contains the base64-encoded content of the file

    // Print or use the base64-encoded data as needed
    printf("Base64-encoded data:\n%s\n", base64Data);

    // Cleanup
    free(fileContent);
    free(base64Data);

    return 0;
}
