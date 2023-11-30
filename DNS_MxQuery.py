'''
Creates an MX query using the dns.resolver module.
Sends the MX query to a DNS server using a UDP socket.
Receives the DNS response.
Extracts the MX server from the DNS response.
Connects to the MX server on the standard SMTP port (25).
'''

import dns.resolver
import socket

def create_mx_query(domain):
    query = dns.message.make_query(domain, dns.rdatatype.MX)
    return query.to_wire()

def extract_mx_server(dns_response):
    response = dns.message.from_wire(dns_response)
    
    # Check if there are any answer records
    if len(response.answer) == 0:
        print("No answer records found in DNS response.")
        return None
    
    # Extract MX server from the first answer record
    mx_server = response.answer[0].exchange.to_text()
    return mx_server

def connect_to_mx_server(mx_server, mx_port):
    try:
        mx_socket = socket.create_connection((mx_server, mx_port))
        print(f"Connected to MX server: {mx_server}")
        return mx_socket
    except Exception as e:
        print(f"Failed to connect to MX server: {mx_server}. Error: {e}")
        return None

def main():
    domain = "example.com"  # Replace with the domain you want to query
    mx_port = 25  # Standard SMTP port for email servers

    # Generate MX query
    mx_query = create_mx_query(domain)

    # Send the MX query to a DNS server
    dns_server = "8.8.8.8"  # Replace with your DNS server address
    dns_port = 53
    dns_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dns_socket.sendto(mx_query, (dns_server, dns_port))

    # Receive the DNS response
    dns_response, _ = dns_socket.recvfrom(1024)  # Adjust buffer size based on expected response size

    # Parse DNS response and extract MX server
    mx_server = extract_mx_server(dns_response)

    if mx_server:
        # Connect to the MX server
        mx_socket = connect_to_mx_server(mx_server, mx_port)
        if mx_socket:
            # Perform further actions as needed on the connected socket (e.g., send email)

            # Close the socket when done
            mx_socket.close()

    # Close the DNS socket
    dns_socket.close()

if __name__ == "__main__":
    main()
