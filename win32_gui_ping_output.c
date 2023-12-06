#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

// Global variables
HWND g_hTextBox;
HWND g_hButton;
HWND g_hIPAddressInput;

// Function to append text to the textbox
void AppendText(const TCHAR* text) {
    int len = GetWindowTextLength(g_hTextBox);
    SendMessage(g_hTextBox, EM_SETSEL, len, len);
    SendMessage(g_hTextBox, EM_REPLACESEL, 0, (LPARAM)text);
}

// Function to perform ping operation and display the result
void PingIPAddress() {
    TCHAR ipAddress[16];
    GetWindowText(g_hIPAddressInput, ipAddress, sizeof(ipAddress) / sizeof(TCHAR));

    if (_tcslen(ipAddress) == 0) {
        MessageBox(NULL, _T("Please enter an IP address."), _T("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    // Construct the ping command
    TCHAR pingCommand[256];
    _stprintf_s(pingCommand, _T("ping %s"), ipAddress);

    // Open a pipe and execute the ping command
    FILE* pingPipe = _tpopen(pingCommand, _T("r"));
    if (!pingPipe) {
        MessageBox(NULL, _T("Error executing ping command."), _T("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    // Read the ping response
    TCHAR buffer[256];
    while (_fgetts(buffer, sizeof(buffer), pingPipe) != NULL) {
        AppendText(buffer);
    }

    // Close the pipe
    _pclose(pingPipe);
}

// Button click event handler
void OnButtonClick() {
    // Perform the ping operation
    PingIPAddress();
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            // Create IP address input box
            g_hIPAddressInput = CreateWindow(_T("EDIT"), _T(""),
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                10, 10, 120, 20, hwnd, (HMENU)IDC_IPADDRESS_INPUT, GetModuleHandle(NULL), NULL);

            // Create textbox
            g_hTextBox = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""),
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                10, 40, 300, 150, hwnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);

            // Create button
            g_hButton = CreateWindow(_T("BUTTON"), _T("Start"),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10, 200, 100, 30, hwnd, (HMENU)IDC_MAIN_BUTTON, GetModuleHandle(NULL), NULL);
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
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
