#include <windows.h>

/*
This code defines a simple Windows program that creates a main window with a default icon. It also displays a splash screen for 2 seconds using a bitmap resource (specified as IDB_SPLASH) and then creates the main window. Please make sure to replace IDB_SPLASH with the resource ID of your splash image and IDI_ICON with the resource ID of your icon. Also, you need to provide these resources in your project.

To compile this code, you will need to link against user32.lib and include appropriate resource files for the icon and splash image. Additionally, ensure that you have defined the resource IDs (IDB_SPLASH and IDI_ICON) correctly in your resource file.
*/


// Global variables
const char g_szClassName[] = "MyWindowClass";
HBITMAP g_hSplashBitmap = NULL;
HICON g_hIcon = NULL;
HWND g_hSplashWnd = NULL;

// Function to create the main window
BOOL CreateMainWindow(HINSTANCE hInstance) {
    WNDCLASSEX wc;
    HWND hwnd;

    // Register the window class
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = g_hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = g_hIcon;

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    // Create the window
    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, g_szClassName, "My Window", WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    return TRUE;
}

// Function to display the splash screen
void ShowSplashScreen(HINSTANCE hInstance) {
    g_hSplashBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPLASH));
    if (g_hSplashBitmap != NULL) {
        g_hSplashWnd = CreateWindowEx(WS_EX_TOPMOST, "STATIC", NULL, WS_POPUP | WS_VISIBLE,
                                      0, 0, 320, 240, NULL, NULL, hInstance, NULL);
        if (g_hSplashWnd != NULL) {
            SendMessage(g_hSplashWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_hSplashBitmap);
            Sleep(2000); // Display splash for 2 seconds
            DestroyWindow(g_hSplashWnd);
        }
        DeleteObject(g_hSplashBitmap);
    }
}

// WinMain function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MSG Msg;

    // Load the icon
    g_hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));

    // Display splash screen
    ShowSplashScreen(hInstance);

    // Create the main window
    if (!CreateMainWindow(hInstance))
        return FALSE;

    // Message loop
    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return Msg.wParam;
}
