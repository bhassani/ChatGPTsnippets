#include <stdio.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

// Define a custom structure
struct Person {
    char name[50];
    int age;
};

// Function to send a structure over a socket
int sendStructure(SOCKET socket, const struct Person* person) {
    int bytesSent = send(socket, (const char*)person, sizeof(struct Person), 0);
    if (bytesSent == SOCKET_ERROR) {
        fprintf(stderr, "send failed with error: %d\n", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return -1;
    }
    return bytesSent;
}

// Function to receive a structure from a socket
int receiveStructure(SOCKET socket, struct Person* person) {
    int bytesReceived = recv(socket, (char*)person, sizeof(struct Person), 0);
    if (bytesReceived == SOCKET_ERROR) {
        fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return -1;
    }
    return bytesReceived;
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    // Create a socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Set up server address information
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);  // Choose a suitable port
    serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // Loopback address for testing

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "listen failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Accept a connection
    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "accept failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Create a sample Person structure
    struct Person personToSend = {"John Doe", 30};

    // Send the structure over the socket
    if (sendStructure(clientSocket, &personToSend) == -1) {
        return 1;
    }

    // Receive the structure on the other end
    struct Person receivedPerson;
    if (receiveStructure(clientSocket, &receivedPerson) == -1) {
        return 1;
    }

    // Display received data
    printf("Received Person: Name = %s, Age = %d\n", receivedPerson.name, receivedPerson.age);

    // Cleanup
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
