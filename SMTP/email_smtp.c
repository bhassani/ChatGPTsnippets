#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windns.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "dnsapi.lib")

void send_email(SOCKET sock, const char *email) {
    char buffer[1024];
    char date_buffer[128];
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);

    // Format date according to RFC5322
    strftime(date_buffer, sizeof(date_buffer), "%a, %d %b %Y %H:%M:%S +0000", t);

    // Read the server's greeting
    recv(sock, buffer, sizeof(buffer), 0);
    printf("S: %s", buffer);

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
    snprintf(buffer, sizeof(buffer),
             "Date: %s\r\n"
             "From: your-email@example.com\r\n"
             "To: %s\r\n"
             "Subject: Test Email\r\n"
             "\r\n"
             "This is a test email.\r\n"
             ".\r\n",
             date_buffer, email);
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
    struct addrinfo hints, *result;
    DNS_RECORD *dnsRecord;
    DNS_STATUS status;
    char email[256], domain[128];
    char *at_sign;

    printf("Enter email address: ");
    scanf("%s", email);

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

    // Resolve the MX server's hostname to an IP address using getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(dnsRecord->Data.MX.pNameExchange, "25", &hints, &result) != 0) {
        fprintf(stderr, "Failed to resolve MX server.\n");
        DnsRecordListFree(dnsRecord, DnsFreeRecordList);
        WSACleanup();
        return 1;
    }

    // Create socket
    sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed.\n");
        freeaddrinfo(result);
        DnsRecordListFree(dnsRecord, DnsFreeRecordList);
        WSACleanup();
        return 1;
    }

    // Connect to the SMTP server
    if (connect(sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        fprintf(stderr, "Connection to SMTP server failed.\n");
        closesocket(sock);
        freeaddrinfo(result);
        DnsRecordListFree(dnsRecord, DnsFreeRecordList);
        WSACleanup();
        return 1;
    }

    printf("Connected to SMTP server.\n");

    // Send email according to RFC821
    send_email(sock, email);

    // Cleanup
    closesocket(sock);
    freeaddrinfo(result);
    DnsRecordListFree(dnsRecord, DnsFreeRecordList);
    WSACleanup();

    return 0;
}
