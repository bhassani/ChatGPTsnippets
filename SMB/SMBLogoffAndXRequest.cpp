
// SMB LogoffAndXRequest structure
struct SMBLogoffAndXRequest {
    uint8_t WordCount;
    uint16_t AndXCommand;
    uint16_t AndXOffset;
    uint16_t Reserved;
    // Add other fields as needed
};
