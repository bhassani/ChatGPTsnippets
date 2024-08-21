#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 8192

// HTML page served by the server
const char* uploadPage =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n\r\n"
"<html>"
"<body>"
"<h2>Upload a File</h2>"
"<form method=\"POST\" enctype=\"multipart/form-data\">"
"File: <input type=\"file\" name=\"file\"><br>"
"Save as: <input type=\"text\" name=\"filename\"><br>"
"<input type=\"submit\" value=\"Upload\">"
"</form>"
"</body>"
"</html>";

void handle_client(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = 0;

        // Check if it's a GET request
        if (strncmp(buffer, "GET", 3) == 0) {
            send(clientSocket, uploadPage, strlen(uploadPage), 0);
        }
        // Check if it's a POST request
        else if (strncmp(buffer, "POST", 4) == 0) {
            char *boundary = strstr(buffer, "boundary=");
            if (boundary) {
                boundary += 9; // Move past "boundary="
                char *filenameField = strstr(buffer, "name=\"filename\"");
                if (filenameField) {
                    // Move to the filename value
                    char *filenameStart = strstr(filenameField, "\r\n\r\n") + 4;
                    char *filenameEnd = strstr(filenameStart, "\r\n");
                    *filenameEnd = 0; // Null-terminate the filename string

                    // Move to the file data
                    char *fileField = strstr(buffer, "name=\"file\"");
                    if (fileField) {
                        char *fileDataStart = strstr(fileField, "\r\n\r\n") + 4;
                        char *fileDataEnd = strstr(fileDataStart, boundary) - 4;

                        // Open the file for writing
                        FILE *file = fopen(filenameStart, "wb");
                        if (file) {
                            fwrite(fileDataStart, 1, fileDataEnd - fileDataStart, file);
                            fclose(file);
                            send(clientSocket, "HTTP/1.1 200 OK\r\n\r\nFile uploaded successfully", 47, 0);
                        } else {
                            send(clientSocket, "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to open file", 57, 0);
                        }
                    }
                }
            }
        }
    }
    closesocket(clientSocket);
}

int main() {
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in server, client;
    int c;

    // Initialize Winsock
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        return 1;
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    // Bind
    if (bind(serverSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        return 1;
    }

    // Listen to incoming connections
    listen(serverSocket, 3);

    printf("Server started on port 8080\n");

    // Accept incoming connections
    c = sizeof(struct sockaddr_in);
    while ((clientSocket = accept(serverSocket, (struct sockaddr*)&client, &c)) != INVALID_SOCKET) {
        handle_client(clientSocket);
    }

    if (clientSocket == INVALID_SOCKET) {
        printf("Accept failed: %d\n", WSAGetLastError());
        return 1;
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}