#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Callback function to handle the response from the HTTP request
size_t write_callback(void *contents, size_t size, size_t nmemb, char **output) {
    size_t realsize = size * nmemb;
    *output = realloc(*output, realsize + 1);

    if (*output == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    memcpy(*output, contents, realsize);
    (*output)[realsize] = '\0';

    return realsize;
}

// Function to obtain the public IP address
void get_public_ip(char **ip_address) {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        char *response_data = NULL;

        // URL of a service that returns the public IP address
        const char *url = "https://api64.ipify.org?format=json";

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "cURL request failed: %s\n", curl_easy_strerror(res));
        } else {
            // Parse the JSON response to obtain the IP address
            // This is a simple example, and you may want to use a JSON parsing library for a more robust solution
            sscanf(response_data, "{\"ip\":\"%[^\"]\"}", *ip_address);
            printf("Public IP Address: %s\n", *ip_address);
        }

        free(response_data);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

int main() {
    char *ip_address = NULL;

    get_public_ip(&ip_address);

    // Cleanup
    free(ip_address);

    return 0;
}
