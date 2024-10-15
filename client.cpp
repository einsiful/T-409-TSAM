// client.cpp

#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include "MessageHandler.h"

volatile bool running = true; // Use a volatile flag to control the listener thread

void listenServer(int serverSocket) {
    char buffer[1025];
    while (running) {
        ssize_t bytesRead = recv(serverSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            if (running) {
                std::cout << "Disconnected from server." << std::endl;
            }
            break;
        }
        buffer[bytesRead] = '\0';
        std::string message = parseFramedMessage(std::string(buffer));

        if (!message.empty()) {
            std::cout << "Server: " << message << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: ./client <server IP> <server port>" << std::endl;
        return 1;
    }

    int serverSocket;
    struct sockaddr_in server_addr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Failed to create socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(std::stoi(argv[2]));
    inet_aton(argv[1], &server_addr.sin_addr);

    if (connect(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to server");
        close(serverSocket);
        return 1;
    }

    std::thread listener(listenServer, serverSocket);

    std::string input;
    while (std::getline(std::cin, input)) {
        if (input.empty()) {
            continue;
        }

        if (input == "EXIT") {
            break;
        }

        std::cout << "Sending: " << input << std::endl;

        // Frame the message before sending
        std::string framedMessage = frameMessage(input);

        // Send the framed message to the server
        ssize_t bytesSent = send(serverSocket, framedMessage.c_str(), framedMessage.length(), 0);
        if (bytesSent < 0) {
            perror("Failed to send message");
            break;
        }
    }

    // Close the socket to unblock the listener thread
    running = false;
    shutdown(serverSocket, SHUT_RDWR); // Shutdown both send and receive operations
    close(serverSocket);

    // Wait for the listener thread to finish
    listener.join();

    std::cout << "Client exited." << std::endl;
    return 0;
}
