import UIKit

class ViewController: UIViewController {
    
    // UI components
    let ipAddressTextField: UITextField = {
        let textField = UITextField()
        textField.placeholder = "Enter IP Address"
        textField.borderStyle = .roundedRect
        textField.translatesAutoresizingMaskIntoConstraints = false
        return textField
    }()
    
    let pingButton: UIButton = {
        let button = UIButton(type: .system)
        button.setTitle("Ping", for: .normal)
        button.addTarget(self, action: #selector(pingButtonTapped), for: .touchUpInside)
        button.translatesAutoresizingMaskIntoConstraints = false
        return button
    }()
    
    let responseTextView: UITextView = {
        let textView = UITextView()
        textView.isEditable = false
        textView.translatesAutoresizingMaskIntoConstraints = false
        return textView
    }()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Setup UI
        setupUI()
    }
    
    func setupUI() {
        view.addSubview(ipAddressTextField)
        view.addSubview(pingButton)
        view.addSubview(responseTextView)
        
        // Constraints for IP Address TextField
        NSLayoutConstraint.activate([
            ipAddressTextField.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 20),
            ipAddressTextField.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 20),
            ipAddressTextField.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -20),
            ipAddressTextField.heightAnchor.constraint(equalToConstant: 40)
        ])
        
        // Constraints for Ping Button
        NSLayoutConstraint.activate([
            pingButton.topAnchor.constraint(equalTo: ipAddressTextField.bottomAnchor, constant: 10),
            pingButton.centerXAnchor.constraint(equalTo: view.centerXAnchor),
            pingButton.heightAnchor.constraint(equalToConstant: 40)
        ])
        
        // Constraints for Response TextView
        NSLayoutConstraint.activate([
            responseTextView.topAnchor.constraint(equalTo: pingButton.bottomAnchor, constant: 20),
            responseTextView.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 20),
            responseTextView.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -20),
            responseTextView.bottomAnchor.constraint(equalTo: view.bottomAnchor, constant: -20)
        ])
    }
    
    @objc func pingButtonTapped() {
        // Get the IP address from the text field
        guard let ipAddress = ipAddressTextField.text else {
            return
        }
        
        // Ping the IP address
        let pingResult = performPing(ipAddress: ipAddress)
        
        // Display the ping result in the text view
        responseTextView.text = pingResult
    }
    
    func performPing(ipAddress: String) -> String {
        // Implement ping logic here
        // You may need to use a library or a system command to perform the ping
        // For simplicity, a placeholder response is used in this example
        return "Ping response for \(ipAddress)"
    }
}
