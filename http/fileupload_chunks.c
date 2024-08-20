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

// Function to extract the boundary from the Content-Type header
char* extract_boundary(const char* buffer) {
    char* boundary_start = strstr(buffer, "boundary=");
    if (boundary_start) {
        boundary_start += 9; // Move past "boundary="
        char* boundary_end = strstr(boundary_start, "\r\n");
        if (boundary_end) {
            *boundary_end = '\0';
        }
        return boundary_start;
    }
    return NULL;
}

// Function to extract the filename from the form data
char* extract_filename(const char* buffer) {
    char* filename_field = strstr(buffer, "name=\"filename\"");
    if (filename_field) {
        // Move to the filename value
        char* filename_start = strstr(filename_field, "\r\n\r\n") + 4;
        char* filename_end = strstr(filename_start, "\r\n");
        if (filename_start && filename_end) {
            *filename_end = '\0'; // Null-terminate the filename string
            return filename_start;
        }
    }
    return NULL;
}

// Function to handle file uploads, processing the file data in chunks
void handle_file_upload(SOCKET clientSocket, const char* boundary, const char* filename) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    FILE* file = fopen(filename, "wb");
    if (!file) {
        send(clientSocket, "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to open file", 57, 0);
        return;
    }

    int boundary_len = strlen(boundary);
    char boundary_end[256];
    snprintf(boundary_end, sizeof(boundary_end), "\r\n--%s--", boundary);

    while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        char* data_start = buffer;
        int data_len = bytesReceived;

        // Check for the end of the boundary
        char* boundary_pos = strstr(buffer, boundary_end);
        if (boundary_pos) {
            data_len = boundary_pos - buffer;
            fwrite(data_start, 1, data_len, file);
            break;
        }

        fwrite(data_start, 1, data_len, file);
    }

    fclose(file);
    send(clientSocket, "HTTP/1.1 200 OK\r\n\r\nFile uploaded successfully", 47, 0);
}

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
            char* boundary = extract_boundary(buffer);
            char* filename = extract_filename(buffer);

            if (boundary && filename) {
                handle_file_upload(clientSocket, boundary, filename);
            } else {
                send(clientSocket, "HTTP/1.1 400 Bad Request\r\n\r\nInvalid form data", 48, 0);
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
