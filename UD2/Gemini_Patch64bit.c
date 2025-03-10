#include <windows.h>
#include <stdio.h>

typedef NTSTATUS(NTAPI* PNTCONTINUE)(PCONTEXT Context, BOOLEAN RaiseAlert);
typedef NTSTATUS(NTAPI* PKIUSEREXCEPTIONDISPATCHER)(PEXCEPTION_RECORD ExceptionRecord, PCONTEXT Context);

int main() {
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) {
        printf("Failed to get handle to ntdll.dll\n");
        return 1;
    }

    PKIUSEREXCEPTIONDISPATCHER KiUserExceptionDispatcher = (PKIUSEREXCEPTIONDISPATCHER)GetProcAddress(hNtdll, "KiUserExceptionDispatcher");
    PNTCONTINUE NtContinue = (PNTCONTINUE)GetProcAddress(hNtdll, "NtContinue");

    if (!KiUserExceptionDispatcher || !NtContinue) {
        printf("Failed to get addresses of KiUserExceptionDispatcher or NtContinue\n");
        return 1;
    }

    printf("KiUserExceptionDispatcher: %p\n", KiUserExceptionDispatcher);
    printf("NtContinue: %p\n", NtContinue);

    // Prepare the shellcode (64-bit)
    unsigned char shellcode[] = {
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, <NtContinue address>
        0xFF, 0xE0                                                    // jmp rax
    };

    // Replace the placeholder address with the actual address (64-bit)
    *(UINT64*)(shellcode + 2) = (UINT64)NtContinue;

    // Save original bytes
    unsigned char originalBytes[sizeof(shellcode)];
    memcpy(originalBytes, KiUserExceptionDispatcher, sizeof(shellcode));

    // Change memory protection
    DWORD oldProtect;
    if (!VirtualProtect(KiUserExceptionDispatcher, sizeof(shellcode), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        printf("VirtualProtect failed\n");
        return 1;
    }

    // Patch KiUserExceptionDispatcher
    memcpy(KiUserExceptionDispatcher, shellcode, sizeof(shellcode));

    // Execute the patched code (ud2 triggers exception)
    __asm {
        ud2
    }

    // Restore original bytes and protection
    memcpy(KiUserExceptionDispatcher, originalBytes, sizeof(shellcode));

    if (!VirtualProtect(KiUserExceptionDispatcher, sizeof(shellcode), oldProtect, &oldProtect)) {
        printf("VirtualProtect restore failed\n");
        return 1;
    }

    printf("Patch and execution complete\n");
    return 0;
}
