#include <windows.h>
#include <tchar.h>

/*
The WndProc function handles the creation of the window, the button click event, and the destruction of the window.
The WinMain function registers the window class, creates the window, and enters the main message loop.
The OnButtonClick function is called when the "Start" button is clicked and appends text to the textbox using the AppendText function.
The AppendText function appends the provided text to the existing text in the textbox.
*/

// Global variables
HWND g_hTextBox;
HWND g_hButton;

// Function to append text to the textbox
void AppendText(const TCHAR* text) {
    int len = GetWindowTextLength(g_hTextBox);
    SendMessage(g_hTextBox, EM_SETSEL, len, len);
    SendMessage(g_hTextBox, EM_REPLACESEL, 0, (LPARAM)text);
}

// Button click event handler
void OnButtonClick() {
    // Perform the desired operation
    AppendText(_T("Button Clicked!\r\n"));
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            // Create textbox
            g_hTextBox = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""),
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                10, 10, 300, 200, hwnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);

            // Create button
            g_hButton = CreateWindow(_T("BUTTON"), _T("Start"),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10, 220, 100, 30, hwnd, (HMENU)IDC_MAIN_BUTTON, GetModuleHandle(NULL), NULL);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_MAIN_BUTTON && HIWORD(wParam) == BN_CLICKED) {
                OnButtonClick();
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register the window class
    const TCHAR szWindowClass[] = _T("Win32App");
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0,
                        GetModuleHandle(NULL), LoadIcon(NULL, IDI_APPLICATION),
                        LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1),
                        NULL, szWindowClass, LoadIcon(NULL, IDI_APPLICATION) };

    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Win32 Guided Tour"), NULL);
        return 1;
    }

    // Create the window
    HWND hwnd = CreateWindow(szWindowClass, _T("Win32 Guided Tour Application"), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, NULL, NULL, GetModuleHandle(NULL), NULL);

    if (!hwnd) {
        MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Win32 Guided Tour"), NULL);
        return 1;
    }

    // Show the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Main message loop
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
