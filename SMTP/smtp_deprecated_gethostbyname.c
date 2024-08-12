#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windns.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "dnsapi.lib")

void send_email(SOCKET sock, const char *email) {
    char buffer[1024];

    // Send EHLO
    snprintf(buffer, sizeof(buffer), "EHLO localhost\r\n");
    send(sock, buffer, strlen(buffer), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("S: %s", buffer);

    // Send MAIL FROM command
    snprintf(buffer, sizeof(buffer), "MAIL FROM:<your-email@example.com>\r\n");
    send(sock, buffer, strlen(buffer), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("S: %s", buffer);

    // Send RCPT TO command
    snprintf(buffer, sizeof(buffer), "RCPT TO:<%s>\r\n", email);
    send(sock, buffer, strlen(buffer), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("S: %s", buffer);

    // Send DATA command
    snprintf(buffer, sizeof(buffer), "DATA\r\n");
    send(sock, buffer, strlen(buffer), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("S: %s", buffer);

    // Send email content
    snprintf(buffer, sizeof(buffer), "Subject: Test Email\r\n\r\nThis is a test email.\r\n.\r\n");
    send(sock, buffer, strlen(buffer), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("S: %s", buffer);

    // Send QUIT command
    snprintf(buffer, sizeof(buffer), "QUIT\r\n");
    send(sock, buffer, strlen(buffer), 0);
    recv(sock, buffer, sizeof(buffer), 0);
    printf("S: %s", buffer);
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server;
    struct hostent *host;
    DNS_RECORD *dnsRecord;
    DNS_STATUS status;
    char email[256], domain[128];
    char *at_sign;

    printf("Enter email address: ");
    scanf("%255s", email);

    // Extract domain from email address
    at_sign = strchr(email, '@');
    if (at_sign == NULL) {
        fprintf(stderr, "Invalid email address.\n");
        return 1;
    }
    strcpy(domain, at_sign + 1);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return 1;
    }

    // Query DNS for MX record
    status = DnsQuery_A(domain, DNS_TYPE_MX, DNS_QUERY_STANDARD, NULL, &dnsRecord, NULL);
    if (status != 0) {
        fprintf(stderr, "DNS lookup failed with error: %d\n", status);
        WSACleanup();
        return 1;
    }

    // Get MX record
    if (dnsRecord->wType != DNS_TYPE_MX) {
        fprintf(stderr, "No MX record found for domain %s.\n", domain);
        DnsRecordListFree(dnsRecord, DnsFreeRecordList);
        WSACleanup();
        return 1;
    }

    printf("MX Record found: %s\n", dnsRecord->Data.MX.pNameExchange);

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed.\n");
        DnsRecordListFree(dnsRecord, DnsFreeRecordList);
        WSACleanup();
        return 1;
    }

    // Get IP address of MX server
    host = gethostbyname(dnsRecord->Data.MX.pNameExchange);
    if (host == NULL) {
        fprintf(stderr, "Failed to resolve MX server.\n");
        closesocket(sock);
        DnsRecordListFree(dnsRecord, DnsFreeRecordList);
        WSACleanup();
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(25);
    server.sin_addr.s_addr = *((unsigned long *)host->h_addr);

    // Connect to SMTP server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Connection to SMTP server failed.\n");
        closesocket(sock);
        DnsRecordListFree(dnsRecord, DnsFreeRecordList);
        WSACleanup();
        return 1;
    }

    printf("Connected to SMTP server.\n");

    // Send email according to RFC821
    send_email(sock, email);

    // Cleanup
    closesocket(sock);
    DnsRecordListFree(dnsRecord, DnsFreeRecordList);
    WSACleanup();

    return 0;
}
