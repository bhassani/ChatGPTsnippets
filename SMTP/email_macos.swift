/*
Please note that macOS has restrictions on directly sending emails through low-level protocols like SMTP due to security and privacy concerns. Sending emails is usually done using the MFMailComposeViewController for GUI applications or through a dedicated email sending service.

However, if you are dealing with a command-line script and need to interact with an SMTP server directly, you might consider using a third-party library like SwiftSMTP for a higher-level interface.
*/

import Foundation

func getLocalDNSServer() -> String? {
    let task = Process()
    task.launchPath = "/usr/bin/env"
    task.arguments = ["scutil", "--dns"]
    
    let pipe = Pipe()
    task.standardOutput = pipe
    task.launch()
    
    let data = pipe.fileHandleForReading.readDataToEndOfFile()
    if let output = String(data: data, encoding: .utf8) {
        let lines = output.components(separatedBy: "\n")
        for line in lines {
            if line.contains("nameserver") {
                let components = line.components(separatedBy: ":")
                if components.count > 1 {
                    return components[1].trimmingCharacters(in: .whitespaces)
                }
            }
        }
    }
    return nil
}

func performMXQuery(domain: String, dnsServer: String) -> [String] {
    let task = Process()
    task.launchPath = "/usr/bin/env"
    task.arguments = ["nslookup", "-type=mx", domain, dnsServer]
    
    let pipe = Pipe()
    task.standardOutput = pipe
    task.launch()
    
    let data = pipe.fileHandleForReading.readDataToEndOfFile()
    if let output = String(data: data, encoding: .utf8) {
        let mxRecords = output.components(separatedBy: "\n").filter { $0.contains("mail exchanger") }
        return mxRecords.map { $0.components(separatedBy: " ").last ?? "" }
    }
    return []
}

func connectToMXServer(mxServer: String, port: Int) -> FileHandle? {
    let socket = Socket()
    do {
        try socket.connect(to: mxServer, port: port)
        return socket.fileHandleForReading
    } catch {
        print("Error connecting to MX server: \(error)")
        return nil
    }
}

func sendSMTPCommands(socket: FileHandle, sender: String, recipient: String, subject: String, body: String, attachmentPath: String) {
    // Implement SMTP commands here
    // Example: EHLO, MAIL FROM, RCPT TO, DATA, etc.
    
    // Read file content and convert to base64
    if let fileData = try? Data(contentsOf: URL(fileURLWithPath: attachmentPath)),
       let base64Encoded = fileData.base64EncodedString(options: .lineLength64Characters) {
        
        // Compose the email body with attachment
        let emailBody = "This is the email body."
        let emailWithAttachment = createEmailWithAttachment(body: emailBody, attachment: base64Encoded)
        
        // Send the email body over the socket
        if let data = emailWithAttachment.data(using: .utf8) {
            socket.write(data)
        }
    }
    
    // Close the socket
    socket.closeFile()
}

func createEmailWithAttachment(body: String, attachment: String) -> String {
    // Compose the email with attachment
    return """
    From: your_username@example.com
    To: recipient@example.com
    Subject: Test Email
    MIME-Version: 1.0
    Content-Type: multipart/mixed; boundary=boundarystring
    
    --boundarystring
    Content-Type: text/plain
    
    \(body)
    
    --boundarystring
    Content-Type: application/octet-stream
    Content-Transfer-Encoding: base64
    Content-Disposition: attachment; filename="attachment.txt"
    
    \(attachment)
    --boundarystring--
    """
}

// Example usage
if let dnsServer = getLocalDNSServer() {
    print("Local DNS Server: \(dnsServer)")
    
    let mxServers = performMXQuery(domain: "example.com", dnsServer: dnsServer)
    print("MX Servers: \(mxServers)")
    
    if let mxServer = mxServers.first,
       let socket = connectToMXServer(mxServer: mxServer, port: 25) {
        
        sendSMTPCommands(socket: socket, sender: "your_username@example.com", recipient: "recipient@example.com", subject: "Test Email", body: "This is a test email.", attachmentPath: "path/to/attachment.txt")
    }
} else {
    print("Failed to retrieve local DNS server.")
}
