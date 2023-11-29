#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

/*
This example uses the getaddrinfo function to perform a DNS query for the specified domain. 
The ai_canonname field of the returned addrinfo structure contains the canonical name of the queried domain. 
The loop iterates through the results and prints the canonical name and the first MX record found.

Make sure to replace the domain variable with the desired domain for which you want to query MX records.
Note: This example assumes that the domain has MX records and that the DNS resolution is successful. Error handling and additional checks may be necessary for a production environment.
*/

void mxQuery(const char *domain) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return;
    }

    struct addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    int status = getaddrinfo(domain, NULL, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(status));
        WSACleanup();
        return;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if (rp->ai_canonname != NULL) {
            printf("Canonical Name: %s\n", rp->ai_canonname);
        }

        // Check if this is an MX record
        if (rp->ai_family == AF_INET || rp->ai_family == AF_INET6) {
            printf("Mail Exchange (MX) Record: %s\n", rp->ai_canonname);
            break; // Only consider the first MX record
        }
    }

    freeaddrinfo(result);
    WSACleanup();
}

int main() {
    const char *domain = "example.com"; // Replace with the desired domain
    mxQuery(domain);

    return 0;
}
