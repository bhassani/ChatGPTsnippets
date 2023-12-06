import socket
import dns.resolver

def get_local_dns_server():
    # Use a default DNS server (e.g., Google's public DNS)
    dns_resolver = dns.resolver.Resolver(configure=False)
    dns_resolver.nameservers = ['8.8.8.8']

    # Query for the local DNS server
    local_dns_server = dns_resolver.query(socket.gethostname(), 'A')[0].address
    return local_dns_server

def perform_mx_query(domain):
    # Perform an MX query for the specified domain
    try:
        answers = dns.resolver.query(domain, 'MX')
        mx_records = [(answer.preference, str(answer.exchange)) for answer in answers]
        return sorted(mx_records)
    except dns.resolver.NXDOMAIN:
        return None

def connect_to_mx_server(mx_server):
    # Connect to the MX server on port 25
    mx_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    mx_socket.connect((mx_server, 25))
    return mx_socket

def send_smtp_commands(mx_socket):
    # Issue SMTP commands
    commands = [
        b"EHLO example.com\r\n",
        b"MAIL FROM: <sender@example.com>\r\n",
        b"RCPT TO: <recipient@example.com>\r\n",
        b"DATA\r\n",
        b"Subject: Test Email\r\n\r\n",
        b"This is a test email.\r\n.\r\n",
        b"QUIT\r\n"
    ]

    for command in commands:
        mx_socket.sendall(command)
        response = mx_socket.recv(1024)
        print(response.decode('utf-8').strip())

def main():
    domain_to_query = "example.com"

    # Get the local DNS server
    local_dns_server = get_local_dns_server()
    print(f"Local DNS Server: {local_dns_server}")

    # Perform MX query
    mx_records = perform_mx_query(domain_to_query)
    if mx_records:
        print(f"MX Records for {domain_to_query}:\n{mx_records}")

        # Connect to the first MX server
        mx_server = mx_records[0][1]
        mx_socket = connect_to_mx_server(mx_server)

        # Send SMTP commands
        send_smtp_commands(mx_socket)

        # Close the socket
        mx_socket.close()
    else:
        print(f"No MX records found for {domain_to_query}")

if __name__ == "__main__":
    main()
