#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_ADDR "smtp.your-smtp-server.com"
#define SERVER_PORT 25
#define BUFFER_SIZE 1024

// Function to send a command and receive a response
int sendCommand(SOCKET socket, const char* command) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s\r\n", command);

    if (send(socket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
        printf("Send failed.\n");
        return -1;
    }

    printf("Sent: %s", command);
    return 0;
}

int receiveResponse(SOCKET socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    if (recv(socket, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
        printf("Receive failed.\n");
        return -1;
    }

    printf("Received: %s", buffer);
    return 0;
}

// Function to encode a file in base64
char* base64EncodeFile(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error opening file.\n");
        return NULL;
    }

    // Find the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file content
    char* file_content = (char*)malloc(file_size);
    if (!file_content) {
        fclose(file);
        printf("Memory allocation failed.\n");
        return NULL;
    }

    // Read the file content
    fread(file_content, 1, file_size, file);
    fclose(file);

    // Encode the file content in base64
    char* base64_encoded = (char*)malloc(((file_size + 2) / 3) * 4 + 1);
    if (!base64_encoded) {
        free(file_content);
        printf("Memory allocation failed.\n");
        return NULL;
    }

    DWORD base64_length = 0;
    if (!CryptBinaryToStringA((BYTE*)file_content, file_size, CRYPT_STRING_BASE64, base64_encoded, &base64_length)) {
        free(file_content);
        free(base64_encoded);
        printf("Base64 encoding failed.\n");
        return NULL;
    }

    free(file_content);
    return base64_encoded;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("Error creating socket.\n");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Connect failed.\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    receiveResponse(clientSocket);  // Receive the server's greeting

    // Send EHLO command
    sendCommand(clientSocket, "EHLO example.com");
    receiveResponse(clientSocket);

    // Send MAIL FROM command
    sendCommand(clientSocket, "MAIL FROM: <sender@example.com>");
    receiveResponse(clientSocket);

    // Send RCPT TO command
    sendCommand(clientSocket, "RCPT TO: <recipient@example.com>");
    receiveResponse(clientSocket);

    // Send DATA command
    sendCommand(clientSocket, "DATA");
    receiveResponse(clientSocket);

    // Construct the email with base64-encoded attachment
    char* attachment = base64EncodeFile("attachment.txt");
    if (attachment == NULL) {
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Send email headers
    sendCommand(clientSocket, "Subject: Test Email\r\n");
    sendCommand(clientSocket, "MIME-Version: 1.0\r\n");
    sendCommand(clientSocket, "Content-Type: multipart/mixed; boundary=boundary123456789\r\n");
    sendCommand(clientSocket, "\r\n--boundary123456789\r\n");

    // Send email body
    sendCommand(clientSocket, "Content-Type: text/plain\r\n\r\nThis is the email body.\r\n");
    sendCommand(clientSocket, "\r\n--boundary123456789\r\n");

    // Send base64-encoded attachment
    sendCommand(clientSocket, "Content-Type: application/octet-stream; name=attachment.txt\r\n");
    sendCommand(clientSocket, "Content-Transfer-Encoding: base64\r\n");
    sendCommand(clientSocket, "Content-Disposition: attachment; filename=attachment.txt\r\n\r\n");
    sendCommand(clientSocket, attachment);
    sendCommand(clientSocket, "\r\n--boundary123456789--\r\n");

    // End the email with a period on a new line
    sendCommand(clientSocket, ".");
    receiveResponse(clientSocket);

    // Quit the session
    sendCommand(clientSocket, "QUIT");
    receiveResponse(clientSocket);

    // Close the socket
    closesocket(clientSocket);
    WSACleanup();

    free(attachment);

    return 0;
}
