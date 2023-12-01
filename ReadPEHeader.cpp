#include <stdio.h>
#include <stdint.h>

// DOS Header structure
typedef struct {
    uint16_t e_magic;    // Magic number (MZ)
    uint16_t e_cblp;
    uint16_t e_cp;
    uint16_t e_crlc;
    uint16_t e_cparhdr;
    uint16_t e_minalloc;
    uint16_t e_maxalloc;
    uint16_t e_ss;
    uint16_t e_sp;
    uint16_t e_csum;
    uint16_t e_ip;
    uint16_t e_cs;
    uint16_t e_lfarlc;
    uint16_t e_ovno;
    uint16_t e_res[4];
    uint16_t e_oemid;
    uint16_t e_oeminfo;
    uint16_t e_res2[10];
    int32_t e_lfanew;     // File address of new exe header (PE Signature)
} IMAGE_DOS_HEADER;

// PE Signature ("PE\0\0")
#define PE_SIGNATURE 0x4550

// COFF Header structure
typedef struct {
    uint16_t machine;
    uint16_t numberOfSections;
    uint32_t timeDateStamp;
    uint32_t pointerToSymbolTable;
    uint32_t numberOfSymbols;
    uint16_t sizeOfOptionalHeader;
    uint16_t characteristics;
} IMAGE_FILE_HEADER;

// Optional Header Magic values
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10B
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20B

// Optional Header structure
typedef struct {
    uint16_t magic;
    uint8_t majorLinkerVersion;
    uint8_t minorLinkerVersion;
    uint32_t sizeOfCode;
    uint32_t sizeOfInitializedData;
    uint32_t sizeOfUninitializedData;
    uint32_t addressOfEntryPoint;
    uint32_t baseOfCode;
    uint32_t baseOfData;  // Only present in 32-bit optional header
    // ... (other fields omitted for simplicity)
} IMAGE_OPTIONAL_HEADER32;

int main() {
    FILE *file;
    IMAGE_DOS_HEADER dosHeader;
    IMAGE_FILE_HEADER fileHeader;
    IMAGE_OPTIONAL_HEADER32 optionalHeader32;

    // Open the PE file in binary mode
    file = fopen("path/to/your/pe/file.exe", "rb");

    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Read DOS Header
    fread(&dosHeader, sizeof(IMAGE_DOS_HEADER), 1, file);

    // Check the PE signature
    if (dosHeader.e_magic != PE_SIGNATURE) {
        printf("Not a valid PE file.\n");
        fclose(file);
        return 1;
    }

    // Seek to the PE Signature (e_lfanew)
    fseek(file, dosHeader.e_lfanew, SEEK_SET);

    // Read COFF Header
    fread(&fileHeader, sizeof(IMAGE_FILE_HEADER), 1, file);

    // Read Optional Header (32-bit)
    fread(&optionalHeader32, sizeof(IMAGE_OPTIONAL_HEADER32), 1, file);

    // Display some information
    printf("Machine: 0x%X\n", fileHeader.machine);
    printf("Number of Sections: %d\n", fileHeader.numberOfSections);
    printf("Size of Code: %u\n", optionalHeader32.sizeOfCode);
    printf("Address of Entry Point: 0x%X\n", optionalHeader32.addressOfEntryPoint);

    // Close the file
    fclose(file);

    return 0;
}
