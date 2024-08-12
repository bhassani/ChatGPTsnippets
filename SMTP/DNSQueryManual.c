#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define DNS_PORT 53
#define BUFFER_SIZE 512

// DNS header structure
struct DNS_HEADER {
    unsigned short id; // Identification number

    unsigned char rd :1; // Recursion Desired
    unsigned char tc :1; // Truncated Message
    unsigned char aa :1; // Authoritative Answer
    unsigned char opcode :4; // Purpose of message
    unsigned char qr :1; // Query/Response Flag

    unsigned char rcode :4; // Response Code
    unsigned char cd :1; // Checking Disabled
    unsigned char ad :1; // Authenticated Data
    unsigned char z :1; // Reserved
    unsigned char ra :1; // Recursion Available

    unsigned short q_count; // Number of questions
    unsigned short ans_count; // Number of answer entries
    unsigned short auth_count; // Number of authority entries
    unsigned short add_count; // Number of resource entries
};

// DNS question structure
struct QUESTION {
    unsigned short qtype;
    unsigned short qclass;
};

// Resource record data
#pragma pack(push, 1)
struct R_DATA {
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)

// Pointers to resource record contents
struct RES_RECORD {
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
};

// Function to change www.google.com to 3www6google3com format
void ChangetoDnsNameFormat(unsigned char *dns, unsigned char *host) {
    int lock = 0, i;
    strcat((char*)host, ".");

    for(i = 0; i < strlen((char*)host); i++) {
        if(host[i] == '.') {
            *dns++ = i - lock;
            for(; lock < i; lock++) {
                *dns++ = host[lock];
            }
            lock++; // or lock = i+1;
        }
    }
    *dns++ = '\0';
}

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in dest;
    unsigned char buf[BUFFER_SIZE], *qname;
    struct DNS_HEADER *dns = NULL;
    struct QUESTION *qinfo = NULL;
    int i;

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d", WSAGetLastError());
        return 1;
    }
    printf("Winsock initialized.\n");

    // Create a UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // UDP packet for DNS queries
    if (sock == INVALID_SOCKET) {
        printf("Could not create socket: %d", WSAGetLastError());
        return 1;
    }

    dest.sin_family = AF_INET;
    dest.sin_port = htons(DNS_PORT);
    dest.sin_addr.s_addr = inet_addr("8.8.8.8"); // Google's DNS server

    // Input email address
    char email[100], domain[100];
    printf("Enter email address: ");
    scanf("%s", email);

    // Strip domain from email address
    char *at = strchr(email, '@');
    if (!at) {
        printf("Invalid email address.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    strcpy(domain, at + 1);
    printf("Domain extracted: %s\n", domain);

    // Set up the DNS header
    dns = (struct DNS_HEADER *)&buf;

    dns->id = (unsigned short) htons(GetCurrentProcessId());
    dns->qr = 0; // This is a query
    dns->opcode = 0; // Standard query
    dns->aa = 0; // Not Authoritative
    dns->tc = 0; // This message is not truncated
    dns->rd = 1; // Recursion Desired
    dns->ra = 0; // Recursion not available! This is a query
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1); // We have only 1 question
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;

    // Point to the query portion
    qname = (unsigned char*)&buf[sizeof(struct DNS_HEADER)];
    ChangetoDnsNameFormat(qname, (unsigned char*)domain);

    qinfo = (struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)];
    qinfo->qtype = htons(15); // MX Query
    qinfo->qclass = htons(1); // IN

    // Send the DNS query
    if (sendto(sock, (char*)buf, sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION), 0, (struct sockaddr*)&dest, sizeof(dest)) == SOCKET_ERROR) {
        printf("Sendto failed: %d", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("DNS Query sent.\n");

    // Receive the DNS response
    int dest_len = sizeof(dest);
    if (recvfrom(sock, (char*)buf, BUFFER_SIZE, 0, (struct sockaddr*)&dest, &dest_len) == SOCKET_ERROR) {
        printf("Recvfrom failed: %d", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("DNS Response received.\n");

    // Read the answers
    dns = (struct DNS_HEADER*) buf;
    qname = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1) + sizeof(struct QUESTION)];

    for (i = 0; i < ntohs(dns->ans_count); i++) {
        qname = qname + 2 + sizeof(struct R_DATA); // Skip over the name and type fields

        unsigned short preference = ntohs(*(unsigned short*)qname); // MX preference
        qname += 2; // skip preference

        char mx_record[256];
        int pos = 0;
        while (*qname) {
            int len = *qname++;
            while (len--) {
                mx_record[pos++] = *qname++;
            }
            if (*qname) mx_record[pos++] = '.';
        }
        mx_record[pos] = '\0';

        printf("Mail Exchange (MX) server: %s with preference %d\n", mx_record, preference);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
