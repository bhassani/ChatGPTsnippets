#include <windows.h>
#include <stdio.h>

#define JMP_SIZE 5  // Size of the jump instruction

// Function prototype for the original function (e.g., target function)
typedef void (*OriginalFunctionType)(void);
OriginalFunctionType OriginalFunction;

// Memory to store the original bytes
BYTE originalBytes[JMP_SIZE];

// Hooked function
void HookedFunction() {
    printf("Hooked Function Called!\n");
    // Optionally, call the original function
    // Unhook first to avoid infinite loop
    memcpy((void*)OriginalFunction, originalBytes, JMP_SIZE);
    OriginalFunction();
    HookFunction((void*)OriginalFunction, (void*)HookedFunction);
}

// Detour function to hook
void HookFunction(void* targetFunction, void* hookFunction) {
    DWORD oldProtect;

    // Save original bytes
    memcpy(originalBytes, targetFunction, JMP_SIZE);

    // Calculate the relative jump offset
    DWORD relativeAddress = (DWORD)hookFunction - (DWORD)targetFunction - JMP_SIZE;

    // Change memory protection to allow writing
    VirtualProtect(targetFunction, JMP_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);

    // Write jump instruction
    BYTE jmpInstruction[JMP_SIZE] = { 0xE9 };  // JMP opcode
    *(DWORD*)(jmpInstruction + 1) = relativeAddress;

    memcpy(targetFunction, jmpInstruction, JMP_SIZE);

    // Restore memory protection
    VirtualProtect(targetFunction, JMP_SIZE, oldProtect, &oldProtect);
}

// A sample target function to hook
void TargetFunction() {
    printf("Original Target Function!\n");
}

int main() {
    // Point to the target function
    OriginalFunction = (OriginalFunctionType)TargetFunction;

    printf("Before Hooking:\n");
    TargetFunction();

    // Hook the target function
    HookFunction((void*)TargetFunction, (void*)HookedFunction);

    printf("\nAfter Hooking:\n");
    TargetFunction();

    // Optionally restore original function to call it directly
    memcpy((void*)TargetFunction, originalBytes, JMP_SIZE);

    printf("\nAfter Unhooking:\n");
    TargetFunction();

    return 0;
}
