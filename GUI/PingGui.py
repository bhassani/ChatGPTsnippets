import tkinter as tk
from tkinter import scrolledtext
import subprocess

def ping_ip():
    # Get the IP address from the entry widget
    ip_address = entry_ip.get()

    if not ip_address:
        # If no IP address is provided, show an error message
        result_text.config(state=tk.NORMAL)
        result_text.delete(1.0, tk.END)
        result_text.insert(tk.END, "Please enter an IP address.")
        result_text.config(state=tk.DISABLED)
        return

    # Construct the ping command
    ping_command = ["ping", ip_address]

    try:
        # Execute the ping command and capture the output
        result = subprocess.run(ping_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        # Display the ping result in the text widget
        result_text.config(state=tk.NORMAL)
        result_text.delete(1.0, tk.END)
        result_text.insert(tk.END, result.stdout)
        result_text.insert(tk.END, result.stderr)
        result_text.config(state=tk.DISABLED)
    except Exception as e:
        # Handle exceptions, e.g., if the ping command fails
        result_text.config(state=tk.NORMAL)
        result_text.delete(1.0, tk.END)
        result_text.insert(tk.END, f"Error: {str(e)}")
        result_text.config(state=tk.DISABLED)

# Create the main window
window = tk.Tk()
window.title("Ping Tool")

# IP Address entry
label_ip = tk.Label(window, text="Enter IP Address:")
label_ip.grid(row=0, column=0, padx=10, pady=10)
entry_ip = tk.Entry(window)
entry_ip.grid(row=0, column=1, padx=10, pady=10)

# Start button
button_start = tk.Button(window, text="Start", command=ping_ip)
button_start.grid(row=0, column=2, padx=10, pady=10)

# Result text box
result_text = scrolledtext.ScrolledText(window, wrap=tk.WORD, width=60, height=10)
result_text.grid(row=1, column=0, columnspan=3, padx=10, pady=10)
result_text.config(state=tk.DISABLED)

# Run the GUI
window.mainloop()
