#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DNS header structure
#pragma pack(push, 1)
struct DNSHeader {
    USHORT id;
    USHORT flags;
    USHORT qdcount;
    USHORT ancount;
    USHORT nscount;
    USHORT arcount;
};
#pragma pack(pop)

// DNS question structure
struct DNSQuestion {
    USHORT qtype;
    USHORT qclass;
};

// DNS resource record structure
struct DNSResourceRecord {
    USHORT type;
    USHORT _class;
    ULONG ttl;
    USHORT rdlength;
};

// Function to convert domain name to DNS format
void convertDomainToDNSFormat(char* dns, const char* domain) {
    int i, j = 0;
    strcat(domain, ".");

    for (i = 0; i < strlen(domain); i++) {
        if (domain[i] == '.') {
            dns[j++] = i - j;
            while (j < i) {
                dns[j] = domain[j++];
            }
        }
    }
    dns[j++] = '\0';
}

// Function to create an MX query
void createMXQuery(char* dnsQuery, const char* domain) {
    // DNS header
    struct DNSHeader* dnsHeader = (struct DNSHeader*)dnsQuery;
    dnsHeader->id = (USHORT)htons(0x1234); // Example ID
    dnsHeader->flags = htons(0x0100);      // Standard query
    dnsHeader->qdcount = htons(1);         // Number of questions

    // DNS question
    char* dnsQuestion = dnsQuery + sizeof(struct DNSHeader);
    convertDomainToDNSFormat(dnsQuestion, domain);

    struct DNSQuestion* dnsQues = (struct DNSQuestion*)(dnsQuery + sizeof(struct DNSHeader) + strlen(dnsQuestion) + 1);
    dnsQues->qtype = htons(0x000f); // MX record type
    dnsQues->qclass = htons(0x0001); // Internet class
}

// Function to parse DNS response and extract MX server
int parseDNSResponse(const char* dnsResponse, size_t responseSize, char* mxServer, size_t mxServerSize) {
    struct DNSHeader* dnsHeader = (struct DNSHeader*)dnsResponse;

    // Check if it's a valid DNS response
    if (ntohs(dnsHeader->ancount) == 0) {
        printf("No answer records found in DNS response.\n");
        return -1;
    }

    // Move to the first answer record
    const char* answerRecord = dnsResponse + sizeof(struct DNSHeader) + strlen(dnsResponse + sizeof(struct DNSHeader) + 1) + 1 + sizeof(struct DNSQuestion);

    // Extract MX server from the first answer record
    struct DNSResourceRecord* dnsRR = (struct DNSResourceRecord*)answerRecord;
    const char* mxServerName = answerRecord + sizeof(struct DNSResourceRecord);
    size_t mxServerNameLength = strlen(mxServerName) + 1;

    if (mxServerNameLength > mxServerSize) {
        printf("MX server name is too long.\n");
        return -1;
    }

    strncpy(mxServer, mxServerName, mxServerNameLength);

    return 0;
}

// Function to connect to the MX server
SOCKET connectToMXServer(const char* mxServer, int mxPort) {
    SOCKET mxSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (mxSocket == INVALID_SOCKET) {
        perror("Socket creation failed");
        return INVALID_SOCKET;
    }

    struct sockaddr_in mxServerAddr;
    mxServerAddr.sin_family = AF_INET;
    mxServerAddr.sin_port = htons(mxPort);

    if (inet_pton(AF_INET, mxServer, &(mxServerAddr.sin_addr)) <= 0) {
        perror("Invalid MX server address");
        closesocket(mxSocket);
        return INVALID_SOCKET;
    }

    if (connect(mxSocket, (struct sockaddr*)&mxServerAddr, sizeof(mxServerAddr)) == SOCKET_ERROR) {
        perror("Connection to MX server failed");
        closesocket(mxSocket);
        return INVALID_SOCKET;
    }

    return mxSocket;
}

int main() {
    // Replace "example.com" with the domain you want to query
    const char* domain = "example.com";
    const int mxPort = 25; // Standard SMTP port for email servers

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return 1;
    }

    // Allocate memory for the DNS query
    size_t querySize = sizeof(struct DNSHeader) + strlen(domain) + 2 + sizeof(struct DNSQuestion);
    char* dnsQuery = (char*)malloc(querySize);

    if (dnsQuery == NULL) {
        perror("Memory allocation error");
        return 1;
    }

    // Create the MX query
    createMXQuery(dnsQuery, domain);

    // Create a socket
    SOCKET dnsSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (dnsSocket == INVALID_SOCKET) {
        perror("Socket creation failed");
        free(dnsQuery);
        WSACleanup();
        return 1;
    }

    // Specify DNS server address (replace with your DNS server address)
    struct sockaddr_in dnsServerAddr;
    dnsServerAddr.sin_family = AF_INET;
    dnsServerAddr.sin_port = htons(53); // DNS port
    inet_pton(AF_INET, "8.8.8.8", &(dnsServerAddr.sin_addr));

    // Send the DNS query
    sendto(dnsSocket, dnsQuery, (int)querySize, 0, (struct sockaddr*)&dnsServerAddr, sizeof(dnsServerAddr));

    // Receive the DNS response
    char dnsResponse[1024]; // Adjust buffer size based on expected response size
    int bytesRead = recvfrom(dnsSocket, dnsResponse, sizeof(dnsResponse), 0, NULL, NULL);

    if (bytesRead <= 0) {
        perror("Failed to receive DNS response");
        closesocket(dnsSocket);
        free(dnsQuery);
        WSACleanup();
        return 1;
    }

    // Null-terminate the response buffer to use string functions
    dnsResponse[bytesRead] = '\0';

    // Parse DNS response and extract MX server
    char mxServer[256]; // Adjust buffer size based on expected MX server name length
    if (parseDNSResponse(dnsResponse, bytesRead, mxServer, sizeof(mxServer)) == 0) {
        printf("MX server: %s\n", mxServer);

        // Connect to the MX server
        SOCKET mxSocket = connectToMXServer(mxServer, mxPort);
        if (mxSocket != INVALID_SOCKET) {
            printf("Connected to MX server.\n");

            // Perform further actions as needed on the connected socket (e.g., send email)

            // Close the socket when done
            closesocket(mxSocket);
        }
    }

    // Cleanup
    free(dnsQuery);
    closesocket(dnsSocket);
    WSACleanup();

    return 0;
}
