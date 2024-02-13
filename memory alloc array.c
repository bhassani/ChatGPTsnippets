unsigned char *filename[5];
    
*(unsigned long*)&filename[1] = malloc(100);
*(unsigned long*)&filename[2] = malloc(100);
    
memset(filename[1], 0xFF, 100);
memset(filename[2], 0xCC, 100);
    
*(DWORD*)filename[2] = 0x50D800;
    
hexDump(NULL, filename[1], 100);
hexDump(NULL, filename[2], 100);
    
free(*(unsigned long*)&filename[1]);
free(*(unsigned long*)&filename[2]);
