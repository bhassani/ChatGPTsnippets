#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

void sendAimMessage(const char *aimServer, const char *username, const char *password, const char *recipient, const char *message) {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return;
    }

    // Create a socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\n");
        WSACleanup();
        return;
    }

    // Set up the server address
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5190); // AIM server port
    serverAddr.sin_addr.s_addr = inet_addr(aimServer);

    // Connect to the AIM server
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Connection to AIM server failed\n");
        closesocket(sock);
        WSACleanup();
        return;
    }

    // Send login request
    char loginRequest[1024];
    sprintf(loginRequest, "FLAPON\r\n\r\n%c%c%c%c%c%s%c%c%s%c%c%s\r\n",
            0, 0, 0, 1, strlen(username), username, 0, 0, password, 0, 0, "english");

    send(sock, loginRequest, strlen(loginRequest), 0);

    // Send message
    char sendMessage[1024];
    sprintf(sendMessage, "toc_send_im %s %s\r\n", recipient, message);

    send(sock, sendMessage, strlen(sendMessage), 0);

    // Close the socket
    closesocket(sock);

    // Cleanup Winsock
    WSACleanup();
}

int main() {
    const char *aimServer = "login.oscar.aol.com";
    const char *username = "your_username";
    const char *password = "your_password";
    const char *recipient = "recipient_username";
    const char *message = "Hello, AIM!";

    sendAimMessage(aimServer, username, password, recipient, message);

    return 0;
}
