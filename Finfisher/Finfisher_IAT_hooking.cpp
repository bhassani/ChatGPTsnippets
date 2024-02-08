/*
HookCreateWindowEx function is responsible for hooking the CreateWindowEx function. It gets the address of the original function from the user32.dll and then replaces it with the address of the heythere function. Before making this replacement, it changes the protection of the memory page to allow writing.

UnhookCreateWindowEx function restores the original CreateWindowEx function by retrieving its address from user32.dll and setting it back. Before this, it also changes the protection of the memory page.

heythere is the dummy function that will be called whenever CreateWindowEx is invoked after hooking. It just prints a message in this example.
*/

#include <windows.h>
#include <stdio.h>

// Typedefine the original function pointer type
typedef HWND (WINAPI *CREATEWINDOWEXA)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);

// Define the prototype of our dummy function
HWND WINAPI heythere(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    printf("Hey there! This is a dummy function.\n");
    // Perform some dummy operation here if needed
    return NULL; // Or whatever you want to return
}

// Define a global variable to hold the original function address
CREATEWINDOWEXA pOriginalCreateWindowEx = NULL;

// Function to hook CreateWindowEx
BOOL HookCreateWindowEx() {
    // Get the address of the original CreateWindowEx function
    HMODULE hModule = GetModuleHandleA("user32.dll");
    if (hModule != NULL) {
        pOriginalCreateWindowEx = (CREATEWINDOWEXA)GetProcAddress(hModule, "CreateWindowExA");
        if (pOriginalCreateWindowEx != NULL) {
            // Change the protection of the original function's memory page to allow writing
            DWORD dwOldProtect;
            VirtualProtect((LPVOID)&pOriginalCreateWindowEx, sizeof(pOriginalCreateWindowEx), PAGE_EXECUTE_READWRITE, &dwOldProtect);

            // Replace the original function pointer with the address of our dummy function
            pOriginalCreateWindowEx = (CREATEWINDOWEXA)heythere;

            // Restore the protection
            VirtualProtect((LPVOID)&pOriginalCreateWindowEx, sizeof(pOriginalCreateWindowEx), dwOldProtect, &dwOldProtect);
            
            return TRUE;
        }
    }
    return FALSE;
}

// Function to unhook CreateWindowEx
BOOL UnhookCreateWindowEx() {
    // If the original function pointer is not NULL, restore it
    if (pOriginalCreateWindowEx != NULL) {
        // Change the protection of the original function's memory page to allow writing
        DWORD dwOldProtect;
        VirtualProtect((LPVOID)&pOriginalCreateWindowEx, sizeof(pOriginalCreateWindowEx), PAGE_EXECUTE_READWRITE, &dwOldProtect);

        // Restore the original function pointer
        pOriginalCreateWindowEx = (CREATEWINDOWEXA)GetProcAddress(GetModuleHandleA("user32.dll"), "CreateWindowExA");

        // Restore the protection
        VirtualProtect((LPVOID)&pOriginalCreateWindowEx, sizeof(pOriginalCreateWindowEx), dwOldProtect, &dwOldProtect);
        
        return TRUE;
    }
    return FALSE;
}

int main() {
    // Hook the CreateWindowEx function
    if (HookCreateWindowEx()) {
        // Now any call to CreateWindowEx will redirect to our dummy function
        
        // Call CreateWindowEx to see our hook in action
        HWND hWnd = CreateWindowExA(0, "BUTTON", "Hello", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
        
        // Unhook the CreateWindowEx function
        UnhookCreateWindowEx();
    } else {
        printf("Failed to hook CreateWindowEx function.\n");
    }
    
    return 0;
}
