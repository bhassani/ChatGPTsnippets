#include <windows.h>
#include <stdio.h>

LONG WINAPI ExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo) {
    printf("Exception caught! Code: 0x%08X at address: 0x%p\n",
        pExceptionInfo->ExceptionRecord->ExceptionCode,
        pExceptionInfo->ExceptionRecord->ExceptionAddress);

    // Check if the exception is an illegal instruction (EXCEPTION_ILLEGAL_INSTRUCTION)
    if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION) {
        // Determine which instruction caused the exception based on the instruction pointer (RIP/EIP)
        BYTE* faulting_instruction = (BYTE*)pExceptionInfo->ExceptionRecord->ExceptionAddress;
        if (faulting_instruction[0] == 0x0F && faulting_instruction[1] == 0xFF) {
            printf("Caught exception from UD0 instruction.\n");
        } else if (faulting_instruction[0] == 0x0F && faulting_instruction[1] == 0xB9) {
            printf("Caught exception from UD1 instruction.\n");
        } else if (faulting_instruction[0] == 0x0F && faulting_instruction[1] == 0x0B) {
            printf("Caught exception from UD2 instruction.\n");
        }
    }

    // Skip the faulting instruction by advancing the instruction pointer (RIP/EIP)
    pExceptionInfo->ContextRecord->Rip += 2; // For 64-bit systems
    // pExceptionInfo->ContextRecord->Eip += 2; // For 32-bit systems

    return EXCEPTION_CONTINUE_EXECUTION;
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
    // Set up the custom exception handler
    SetupExceptionHandler();

    printf("Triggering an exception with UD0...\n");
    TriggerExceptionUD0();

    printf("Triggering an exception with UD1...\n");
    TriggerExceptionUD1();

    printf("Triggering an exception with UD2...\n");
    TriggerExceptionUD2();

    printf("Execution continued after handling exceptions.\n");

    return 0;
}
