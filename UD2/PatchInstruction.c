#include <stdio.h>
#include <windows.h>

// Define the assembly instructions to replace the beginning of KiUserExceptionDispatcher
unsigned char newInstructions[] = {
    0x68, 0x00, 0x00, 0x00, 0x00,   // PUSH 0x00000000 (address of NTContinue)
    0xC3                            // RET
};

// Function pointer for the original KiUserExceptionDispatcher
typedef LONG(WINAPI *LPKiUserExceptionDispatcher)(
    EXCEPTION_RECORD *ExceptionRecord,
    CONTEXT *ContextRecord
);

int main() {
    // Get a handle to NTDLL module
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll == NULL) {
        printf("Failed to get handle to ntdll.dll\n");
        return 1;
    }

    // Get the address of KiUserExceptionDispatcher
    LPKiUserExceptionDispatcher originalKiUserExceptionDispatcher =
        (LPKiUserExceptionDispatcher)GetProcAddress(ntdll, "KiUserExceptionDispatcher");
    if (originalKiUserExceptionDispatcher == NULL) {
        printf("Failed to get address of KiUserExceptionDispatcher\n");
        return 1;
    }

    // Change the protection of the memory region to allow writing
    DWORD oldProtect;
    if (!VirtualProtect(originalKiUserExceptionDispatcher, sizeof(newInstructions), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        printf("Failed to change memory protection\n");
        return 1;
    }

    //DEV TESTING HERE
    /*
    	GetNtContinueAddress() function retrieves the address of the NtContinue function using GetProcAddress.
	The obtained address is then converted into a suitable form for assembly, which in this case is a PUSH instruction. We use memcpy to copy the address into the PUSH instruction bytes.
	This will print out the address of NtContinue and the corresponding PUSH instruction bytes that can be used in assembly
    */ 
    LPFN_NtContinue pNtContinue = GetNtContinueAddress();
    if (pNtContinue != NULL) {
        printf("Address of NtContinue: 0x%p\n", (void*)pNtContinue);
        
        // Convert the address to a form suitable for pushing onto the stack in assembly
        DWORD address = (DWORD)pNtContinue;
        BYTE pushInstruction[] = { 0x68, 0x00, 0x00, 0x00, 0x00 }; // PUSH address
        memcpy(&pushInstruction[1], &address, sizeof(DWORD));

        // Replace the beginning of KiUserExceptionDispatcher with newInstructions
        memcpy(originalKiUserExceptionDispatcher, pushInstruction, sizeof(pushInstruction));


        printf("PUSH instruction for NtContinue: ");
        for (int i = 0; i < sizeof(pushInstruction); ++i) {
            printf("%02X ", pushInstruction[i]);
        }
        printf("\n");
    }

    //END DEV TESTING

    // Replace the beginning of KiUserExceptionDispatcher with newInstructions
    //memcpy(originalKiUserExceptionDispatcher, newInstructions, sizeof(newInstructions));

    // Restore the original memory protection
    DWORD dummy;
    if (!VirtualProtect(originalKiUserExceptionDispatcher, sizeof(newInstructions), oldProtect, &dummy)) {
        printf("Failed to restore memory protection\n");
        return 1;
    }

    printf("KiUserExceptionDispatcher modified successfully\n");

    return 0;
}


/*
This code uses inline assembly to traverse the export table of ntdll.dll to find the address of the NtContinue function. It starts by retrieving the base address of ntdll.dll, then iterates through the export table to find the address of the NtContinue function by matching its name. Finally, it returns the address of NtContinue. Please note that accessing system internals like this may not be portable across different versions of Windows and may not be guaranteed to work in future versions.
*/

DWORD GetNtContinueAddress() {
    DWORD ntContinueAddress;

    __asm {
        mov eax, fs:[0x30]     // Get the pointer to the PEB
        mov eax, [eax + 0xC]   // Get the pointer to the PEB_LDR_DATA structure
        mov eax, [eax + 0x14]  // Get the pointer to the InMemoryOrderModuleList
        mov eax, [eax]         // Get the first entry in the InMemoryOrderModuleList (which is ntdll)
        mov eax, [eax + 0x10]  // Get the base address of ntdll module

        // Iterate through the export table to find the address of NtContinue
        mov edx, [eax + 0x3C]  // Get the pointer to the IMAGE_NT_HEADERS
        mov edx, [eax + edx + 0x78] // Get the RVA of the export table
        add edx, eax           // Convert RVA to VA

        mov ecx, [edx + 0x18]  // Get the number of functions in the export table
        mov ebx, [edx + 0x20]  // Get the RVA of the export name table
        add ebx, eax           // Convert RVA to VA
        xor esi, esi           // Clear ESI (index variable)

    findFunction:
        inc esi                // Increment index
        mov edi, [ebx + esi * 4] // Get the RVA of the export name
        add edi, eax           // Convert RVA to VA
        mov edi, [edi]         // Get the VA of the export name
        cmp dword ptr [edi], 'etuN' // Check if the first 4 bytes of the export name are 'Nt'
        jnz findFunction       // If not, continue searching

        // Once 'Nt' is found, check if the export name is 'NtContinue'
        cmp dword ptr [edi + 4], 'nutoC' // Check if the next 4 bytes are 'Continue'
        jnz findFunction       // If not, continue searching

        // If 'NtContinue' is found, get its address
        mov ebx, [edx + 0x24] // Get the RVA of the export address table
        add ebx, eax           // Convert RVA to VA
        mov ecx, [ebx + esi * 4] // Get the RVA of NtContinue
        add ecx, eax           // Convert RVA to VA
        mov ntContinueAddress, ecx // Store the address of NtContinue
    }

    return ntContinueAddress;
}

int obtain_NT_Continue() {
    DWORD ntContinueAddress = GetNtContinueAddress();
    printf("Address of NtContinue: 0x%p\n", (void*)ntContinueAddress);
    return 0;
}




/*
GetNtContinueAddress() function retrieves the address of the NtContinue function using GetProcAddress.
The obtained address is then converted into a suitable form for assembly, which in this case is a PUSH instruction. We use memcpy to copy the address into the PUSH instruction bytes.
This will print out the address of NtContinue and the corresponding PUSH instruction bytes that can be used in assembly. Please remember that accessing system internals like this may not be portable across different versions of Windows and may not be guaranteed to work in future versions.
*/


// Declare the prototype of NtContinue
typedef NTSTATUS (NTAPI *LPFN_NtContinue)(
    PCONTEXT ContextRecord,
    BOOLEAN TestAlert
);

// Function to get the address of NtContinue
LPFN_NtContinue GetNtContinueAddress() {
    HMODULE hNtdll = GetModuleHandle("ntdll.dll");
    if (hNtdll == NULL) {
        printf("Failed to get handle to ntdll.dll\n");
        return NULL;
    }

    LPFN_NtContinue pNtContinue = (LPFN_NtContinue)GetProcAddress(hNtdll, "NtContinue");
    if (pNtContinue == NULL) {
        printf("Failed to get address of NtContinue\n");
        return NULL;
    }

    return pNtContinue;
}

int find_address_ntContinue()
{
    LPFN_NtContinue pNtContinue = GetNtContinueAddress();
    if (pNtContinue != NULL) {
        printf("Address of NtContinue: 0x%p\n", (void*)pNtContinue);
        
        // Convert the address to a form suitable for pushing onto the stack in assembly
        DWORD address = (DWORD)pNtContinue;
        BYTE pushInstruction[] = { 0x68, 0x00, 0x00, 0x00, 0x00 }; // PUSH address
        memcpy(&pushInstruction[1], &address, sizeof(DWORD));

        printf("PUSH instruction for NtContinue: ");
        for (int i = 0; i < sizeof(pushInstruction); ++i) {
            printf("%02X ", pushInstruction[i]);
        }
        printf("\n");
    }
    return 0;
}







