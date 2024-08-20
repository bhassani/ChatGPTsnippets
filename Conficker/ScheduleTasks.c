#include <stdio.h>
#include <stdlib.h>

int main() {
    // Remote computer details
    const char* remoteComputer = "\\\\192.168.9.9";  // IP address or hostname of the remote machine
    const char* taskName = "MyRemoteTask";           // Name of the task to create
    const char* taskTime = "12:00";                  // Time to execute the task (HH:MM format)
    const char* executablePath = "C:\\Path\\To\\Your\\Executable.exe";  // Path to the executable you want to run
    
    // Command to create a scheduled task on the remote computer
    char command[1024];
    snprintf(command, sizeof(command),
             "schtasks /Create /S %s /U niceuser /P nicepassword /TN %s /TR \"%s\" /ST %s /SC ONCE",
             remoteComputer, taskName, executablePath, taskTime);

    // Execute the command
    int result = system(command);

    if (result == 0) {
        printf("Task scheduled successfully on remote computer.\n");
    } else {
        printf("Failed to schedule task. Error code: %d\n", result);
    }

    return 0;
}
