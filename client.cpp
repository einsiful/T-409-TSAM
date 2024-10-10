#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    // Check that the correct number of arguments is supplied
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>" << std::endl;
        exit(0);
    }

    char *ip_string = argv[1];
    int port = std::stoi(argv[2]);  // Convert port from string to int

    int sock;

    // Create TCP socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Could not create socket");
        exit(0);
    }

    // Create the server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_string, &server_addr.sin_addr) < 1) {
        std::cerr << "Invalid IP address format" << std::endl;
        exit(0);
    }

    // Connect to the server
    if (connect(sock, (const sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Could not connect");
        exit(0);
    }

    char buffer[BUFFER_SIZE];

    while (true) {
        // Read command from keyboard input
        std::string command;
        std::cout << "Enter command to send to server (or type 'exit' to quit): ";
        std::getline(std::cin, command);

        if (command == "exit") {
            break;  // Exit the loop and terminate the client
        }

        // Append SYS prefix to the command
        std::string sys_command = "SYS " + command;

        // Send the command to the server
        int n = send(sock, sys_command.c_str(), sys_command.length() + 1, 0);
        if (n < sys_command.length() + 1) {
            std::cerr << "Error: Could not send the full command to the server!" << std::endl;
            continue;
        }

        // Receive and print the result from the server
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_received < 0) {
                perror("Error receiving data from server");
                break;
            } else if (bytes_received == 0) {
                // Server closed the connection
                std::cout << "Server closed the connection." << std::endl;
                close(sock);
                exit(0);
            }

            buffer[bytes_received] = '\0';  // Null-terminate the received data
            std::cout << buffer;

            // Check if we received the end of the output
            if (bytes_received < BUFFER_SIZE - 1) {
                break;  // We have received the full response
            }
        }
        std::cout << std::endl;  // Print a newline after the command output
    }

    // Close the socket
    if (close(sock) < 0) {
        perror("Close failed");
        exit(0);
    }

    return 0;
}
