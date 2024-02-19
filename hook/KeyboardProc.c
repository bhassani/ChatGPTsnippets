#include <windows.h>

// Function prototype for the keyboard hook procedure
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

// Global variable to store the handle to the hook
HHOOK g_hHook = NULL;

int main() {
    // Set the global keyboard hook
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);

    if (g_hHook == NULL) {
        MessageBox(NULL, "Failed to set the keyboard hook", "Error", MB_ICONERROR);
        return 1;
    }

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook the keyboard hook before exiting
    UnhookWindowsHookEx(g_hHook);

    return 0;
}

// Keyboard hook procedure
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        // Process the keyboard event
        KBDLLHOOKSTRUCT* pKbdStruct = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN) {
            // Handle key down event
            printf("Key down: %d\n", pKbdStruct->vkCode);
        }
        else if (wParam == WM_KEYUP) {
            // Handle key up event
            printf("Key up: %d\n", pKbdStruct->vkCode);
        }
    }

    // Pass the event to the next hook in the chain
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}
