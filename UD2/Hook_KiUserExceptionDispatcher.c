#include <windows.h>
#include <stdio.h>

// Function pointers to hold the addresses of KiUserExceptionDispatcher and NtContinue
PVOID KiUserExceptionDispatcherAddr = NULL;
PVOID NtContinueAddr = NULL;

// Original bytes of KiUserExceptionDispatcher that we will restore after hooking
BYTE OriginalBytes[16];

// Exception handler function
LONG WINAPI ExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo) {
    printf("Exception caught! Code: 0x%08X at address: 0x%p\n",
        pExceptionInfo->ExceptionRecord->ExceptionCode,
        pExceptionInfo->ExceptionRecord->ExceptionAddress);

    // Skip the faulting instruction by advancing the instruction pointer (RIP/EIP)
    pExceptionInfo->ContextRecord->Rip += 2; // For 64-bit systems
    // pExceptionInfo->ContextRecord->Eip += 2; // For 32-bit systems

    return EXCEPTION_CONTINUE_EXECUTION;
}

// Function to hook KiUserExceptionDispatcher
void HookKiUserExceptionDispatcher() {
    DWORD oldProtect;

    // Save the original bytes of KiUserExceptionDispatcher
    memcpy(OriginalBytes, KiUserExceptionDispatcherAddr, sizeof(OriginalBytes));

    // Change memory protection to allow writing
    VirtualProtect(KiUserExceptionDispatcherAddr, sizeof(OriginalBytes), PAGE_EXECUTE_READWRITE, &oldProtect);

    // Calculate relative jump offset from KiUserExceptionDispatcher to NtContinue
    LONG_PTR relativeAddress = (LONG_PTR)NtContinueAddr - (LONG_PTR)KiUserExceptionDispatcherAddr - 5;

    // Write the JMP instruction
    BYTE jumpInstruction[5] = { 0xE9, (BYTE)(relativeAddress & 0xFF), (BYTE)((relativeAddress >> 8) & 0xFF), (BYTE)((relativeAddress >> 16) & 0xFF), (BYTE)((relativeAddress >> 24) & 0xFF) };
    memcpy(KiUserExceptionDispatcherAddr, jumpInstruction, sizeof(jumpInstruction));

    // Restore original memory protection
    VirtualProtect(KiUserExceptionDispatcherAddr, sizeof(OriginalBytes), oldProtect, &oldProtect);
}

// Function to unhook KiUserExceptionDispatcher
void UnhookKiUserExceptionDispatcher() {
    DWORD oldProtect;

    // Change memory protection to allow writing
    VirtualProtect(KiUserExceptionDispatcherAddr, sizeof(OriginalBytes), PAGE_EXECUTE_READWRITE, &oldProtect);

    // Restore the original bytes of KiUserExceptionDispatcher
    memcpy(KiUserExceptionDispatcherAddr, OriginalBytes, sizeof(OriginalBytes));

    // Restore original memory protection
    VirtualProtect(KiUserExceptionDispatcherAddr, sizeof(OriginalBytes), oldProtect, &oldProtect);
}

void SetupExceptionHandler() {
    AddVectoredExceptionHandler(1, ExceptionHandler);
}

void TriggerExceptionUD0() {
    __asm {
        _emit 0x0F
        _emit 0xFF  // UD0 - Undefined instruction
    }
}

void TriggerExceptionUD1() {
    __asm {
        _emit 0x0F
        _emit 0xB9  // UD1 - Undefined instruction
    }
}

void TriggerExceptionUD2() {
    __asm {
        _emit 0x0F
        _emit 0x0B  // UD2 - Undefined instruction
    }
}

int main() {
    // Resolve addresses of KiUserExceptionDispatcher and NtContinue
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    KiUserExceptionDispatcherAddr = GetProcAddress(ntdll, "KiUserExceptionDispatcher");
    NtContinueAddr = GetProcAddress(ntdll, "NtContinue");

    if (!KiUserExceptionDispatcherAddr || !NtContinueAddr) {
        printf("Failed to resolve addresses!\n");
        return 1;
    }

    // Set up the custom exception handler
    SetupExceptionHandler();

    // Hook KiUserExceptionDispatcher to JMP to NtContinue
    HookKiUserExceptionDispatcher();

    printf("Triggering an exception with UD0...\n");
    TriggerExceptionUD0();

    printf("Triggering an exception with UD1...\n");
    TriggerExceptionUD1();

    printf("Triggering an exception with UD2...\n");
    TriggerExceptionUD2();

    // Unhook KiUserExceptionDispatcher
    UnhookKiUserExceptionDispatcher();

    printf("Execution continued after handling exceptions.\n");

    return 0;
}
