#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define PORT 5557
#define BUFFER_SIZE 1024

// Function declarations
void handleClient(SOCKET clientSocket);
void sendResponse(SOCKET clientSocket, const char* response);
void processCommand(SOCKET clientSocket, const char* command);

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];
    int result;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "Failed to initialize Winsock.\n");
        return 1;
    }

    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Set up the server address structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("FTP server is listening on port %d...\n", PORT);

    while (1) {
        // Accept a client connection
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed: %d\n", WSAGetLastError());
            continue;
        }

        // Send initial response
        sendResponse(clientSocket, "220 OK\r\n");

        // Create a thread to handle the client
        HANDLE clientThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handleClient, (LPVOID)clientSocket, 0, NULL);
        if (clientThread == NULL) {
            fprintf(stderr, "CreateThread failed: %d\n", GetLastError());
            closesocket(clientSocket);
        } else {
            CloseHandle(clientThread);
        }
    }

    // Cleanup
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

void handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    while (1) {
        // Receive data from the client
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            break;
        }

        // Process the received command
        buffer[bytesReceived] = '\0'; // Null-terminate the received data
        processCommand(clientSocket, buffer);
    }

    // Clean up
    closesocket(clientSocket);
}

void sendResponse(SOCKET clientSocket, const char* response) {
    send(clientSocket, response, strlen(response), 0);
}

void processCommand(SOCKET clientSocket, const char* command) {
    if (strncmp(command, "USER", 4) == 0) {
        sendResponse(clientSocket, "331 OK\r\n");
    } else if (strncmp(command, "PASS", 4) == 0) {
        sendResponse(clientSocket, "331 OK\r\n");
    } else if (strncmp(command, "PORT", 4) == 0) {
        sendResponse(clientSocket, "331 OK\r\n");
    } else if (strncmp(command, "RETR", 4) == 0) {
        // Handle file retrieval
        char filename[BUFFER_SIZE];
        sscanf(command, "RETR %s", filename);

        FILE *file = fopen(filename, "rb");
        if (file) {
            // Send file content
            char fileBuffer[BUFFER_SIZE];
            size_t bytesRead;
            while ((bytesRead = fread(fileBuffer, 1, sizeof(fileBuffer), file)) > 0) {
                send(clientSocket, fileBuffer, bytesRead, 0);
            }
            fclose(file);
            sendResponse(clientSocket, "226 Transfer complete\r\n");
        } else {
            sendResponse(clientSocket, "550 File not found\r\n");
        }
    } else if (strncmp(command, "QUIT", 4) == 0) {
        sendResponse(clientSocket, "221 Goodbye\r\n");
    } else {
        sendResponse(clientSocket, "331 OK\r\n");
    }
}
