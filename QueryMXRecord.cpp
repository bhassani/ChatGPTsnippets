#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

void mxDnsQuery(const char *domain, const char *dnsServer) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return;
    }

    struct addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME | AI_ADDRCONFIG | AI_ALL | AI_NUMERICHOST;
    hints.ai_protocol = IPPROTO_TCP;

    int status = getaddrinfo(domain, "0", &hints, &result);
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
        if (rp->ai_family == AF_UNSPEC && rp->ai_protocol == IPPROTO_TCP) {
            printf("Querying MX records for %s using DNS server %s\n", domain, dnsServer);
            char mxQuery[512];
            snprintf(mxQuery, sizeof(mxQuery), "_smtp._tcp.%s", domain); // Construct the MX query

            struct addrinfo mxHints;
            memset(&mxHints, 0, sizeof(struct addrinfo));
            mxHints.ai_family = AF_UNSPEC;
            mxHints.ai_socktype = SOCK_STREAM;
            mxHints.ai_flags = AI_CANONNAME | AI_ADDRCONFIG | AI_ALL | AI_NUMERICHOST;
            mxHints.ai_protocol = IPPROTO_TCP;

            struct addrinfo *mxResult;
            status = getaddrinfo(mxQuery, "0", &mxHints, &mxResult);
            if (status != 0) {
                fprintf(stderr, "getaddrinfo for MX records failed: %s\n", gai_strerror(status));
            } else {
                // Print MX records
                for (struct addrinfo *mxRp = mxResult; mxRp != NULL; mxRp = mxRp->ai_next) {
                    if (mxRp->ai_canonname != NULL) {
                        printf("MX Record: %s\n", mxRp->ai_canonname);
                    }
                }
                freeaddrinfo(mxResult);
            }
            break; // Only consider the first result
        }
    }

    freeaddrinfo(result);
    WSACleanup();
}

int main() {
    const char *domain = "example.com"; // Replace with the desired domain
    const char *dnsServer = "8.8.8.8"; // Replace with the desired DNS server
    mxDnsQuery(domain, dnsServer);

    return 0;
}
