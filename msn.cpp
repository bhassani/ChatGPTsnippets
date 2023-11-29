#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

void sendMsnMessage(const char *msnServer, const char *username, const char *password, const char *recipient, const char *message) {
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
    serverAddr.sin_port = htons(1863); // MSN server port
    serverAddr.sin_addr.s_addr = inet_addr(msnServer);

    // Connect to the MSN server
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Connection to MSN server failed\n");
        closesocket(sock);
        WSACleanup();
        return;
    }

    // Send login request
    char loginRequest[1024];
    sprintf(loginRequest, "VER 1 MSNP8 MSNP9 MSNP10 MSNP11\r\nCVR 1 0x0409 winnt 5.1 i386 MSNMSGR 6.2.0208 MSMSGS %s\r\nUSR 2 %s 0\r\n", username, username);

    send(sock, loginRequest, strlen(loginRequest), 0);

    // Receive response
    char response[1024];
    recv(sock, response, sizeof(response), 0);
    printf("Server Response: %s\n", response);

    // Send message
    char sendMessage[1024];
    sprintf(sendMessage, "MSG %s %s\r\n", recipient, message);

    send(sock, sendMessage, strlen(sendMessage), 0);

    // Close the socket
    closesocket(sock);

    // Cleanup Winsock
    WSACleanup();
}

int main() {
    const char *msnServer = "messenger.hotmail.com";
    const char *username = "your_username@hotmail.com";
    const char *password = "your_password";
    const char *recipient = "recipient@hotmail.com";
    const char *message = "Hello, MSN!";

    sendMsnMessage(msnServer, username, password, recipient, message);

    return 0;
}
