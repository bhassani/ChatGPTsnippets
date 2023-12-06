import socket
import dns.resolver
import base64

def get_local_dns_server():
    # Use a default DNS server (e.g., Google's public DNS)
    dns_resolver = dns.resolver.Resolver(configure=False)
    dns_resolver.nameservers = ['8.8.8.8']

    # Query for the local DNS server
    local_dns_server = dns_resolver.query(socket.gethostname(), 'A')[0].address
    return local_dns_server

def perform_mx_query(domain, dns_server):
    # Perform an MX query for the specified domain using the local DNS server
    try:
        resolver = dns.resolver.Resolver(configure=False)
        resolver.nameservers = [dns_server]
        answers = resolver.query(domain, 'MX')
        mx_records = [(answer.preference, str(answer.exchange)) for answer in answers]
        return sorted(mx_records)
    except dns.resolver.NXDOMAIN:
        return None

def connect_to_mx_server(mx_server):
    # Connect to the MX server on port 25
    mx_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    mx_socket.connect((mx_server, 25))
    return mx_socket

def send_smtp_commands(mx_socket, sender, recipient, subject, body, attachment_path):
    # Issue SMTP commands
    commands = [
        f"EHLO example.com\r\n",
        f"MAIL FROM: <{sender}>\r\n",
        f"RCPT TO: <{recipient}>\r\n",
        "DATA\r\n",
        f"Subject: {subject}\r\n",
        f"{body}\r\n",
        "MIME-Version: 1.0\r\n",
        "Content-Type: multipart/mixed; boundary=boundarystring\r\n",
        "\r\n--boundarystring\r\n",
        "Content-Type: text/plain\r\n",
        f"{body}\r\n",
        "\r\n--boundarystring\r\n",
        "Content-Type: application/octet-stream\r\n",
        "Content-Transfer-Encoding: base64\r\n",
        f"Content-Disposition: attachment; filename=\"attachment.txt\"\r\n",
        "\r\n",
        encode_attachment(attachment_path),
        "\r\n.\r\n",
        "QUIT\r\n"
    ]

    for command in commands:
        mx_socket.sendall(command.encode())
        response = mx_socket.recv(1024)
        print(response.decode('utf-8').strip())

def encode_attachment(attachment_path):
    # Read the file, convert to base64, and format for email attachment
    with open(attachment_path, 'rb') as file:
        attachment_data = base64.b64encode(file.read()).decode('utf-8')
    return attachment_data

def main():
    domain_to_query = "example.com"
    sender_email = "sender@example.com"
    recipient_email = "recipient@example.com"
    email_subject = "Test Email"
    email_body = "This is a test email."

    attachment_path = "path/to/attachment.txt"

    # Get the local DNS server
    local_dns_server = get_local_dns_server()
    print(f"Local DNS Server: {local_dns_server}")

    # Perform MX query
    mx_records = perform_mx_query(domain_to_query, local_dns_server)
    if mx_records:
        print(f"MX Records for {domain_to_query}:\n{mx_records}")

        # Connect to the first MX server
        mx_server = mx_records[0][1]
        mx_socket = connect_to_mx_server(mx_server)

        # Send SMTP commands with email and attachment
        send_smtp_commands(mx_socket, sender_email, recipient_email, email_subject, email_body, attachment_path)

        # Close the socket
        mx_socket.close()
    else:
        print(f"No MX records found for {domain_to_query}")

if __name__ == "__main__":
    main()
