import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class PingApp extends JFrame {
    private JTextField ipAddressField;
    private JTextArea resultTextArea;

    public PingApp() {
        setTitle("Ping Tool");
        setSize(400, 300);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        // Create components
        JLabel ipAddressLabel = new JLabel("Enter IP Address:");
        ipAddressField = new JTextField();
        JButton startButton = new JButton("Start");
        resultTextArea = new JTextArea();
        JScrollPane scrollPane = new JScrollPane(resultTextArea);

        // Set layout
        setLayout(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(5, 5, 5, 5);

        // Add components to the layout
        gbc.gridx = 0;
        gbc.gridy = 0;
        add(ipAddressLabel, gbc);

        gbc.gridx = 1;
        gbc.gridy = 0;
        gbc.fill = GridBagConstraints.HORIZONTAL;
        add(ipAddressField, gbc);

        gbc.gridx = 2;
        gbc.gridy = 0;
        gbc.fill = GridBagConstraints.NONE;
        add(startButton, gbc);

        gbc.gridx = 0;
        gbc.gridy = 1;
        gbc.gridwidth = 3;
        gbc.fill = GridBagConstraints.BOTH;
        gbc.weightx = 1.0;
        gbc.weighty = 1.0;
        add(scrollPane, gbc);

        // Add action listener to the Start button
        startButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                pingIPAddress();
            }
        });
    }

    private void pingIPAddress() {
        // Get the IP address from the text field
        String ipAddress = ipAddressField.getText();

        if (ipAddress.isEmpty()) {
            // If no IP address is provided, show an error message
            resultTextArea.setText("Please enter an IP address.");
            return;
        }

        // Construct the ping command
        String pingCommand = "ping " + ipAddress;

        try {
            // Execute the ping command
            Process process = new ProcessBuilder("cmd.exe", "/c", pingCommand).start();

            // Read the output of the command
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            StringBuilder output = new StringBuilder();
            String line;

            while ((line = reader.readLine()) != null) {
                output.append(line).append("\n");
            }

            // Display the ping result in the text area
            resultTextArea.setText(output.toString());

        } catch (IOException e) {
            e.printStackTrace();
            resultTextArea.setText("Error executing ping command.");
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                new PingApp().setVisible(true);
            }
        });
    }
}
