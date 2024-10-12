#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <fstream>

#define BACKLOG 5
#define BUFFER_SIZE 1024
#define SOH 0x01  // Start of Header (SOH)
#define EOT 0x04  // End of Transmission (EOT)

std::map<int, std::string> clients;
std::vector<int> serverConnections;

void logCommand(const std::string& message) {
    std::ofstream logFile("server.log", std::ios::app);
    logFile << "Timestamp: " << time(0) << " - " << message << std::endl;
}

std::vector<std::string> tokenize(const std::string& input, char delimiter) {
    std::stringstream ss(input);
    std::string token;
    std::vector<std::string> tokens;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int setupServerSocket(int port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Failed to create socket");
        return -1;
    }
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Failed to bind socket");
        return -1;
    }

    if (listen(serverSocket, BACKLOG) < 0) {
        perror("Failed to listen on socket");
        return -1;
    }

    std::cout << "Server listening on port " << port << std::endl;
    return serverSocket;
}

void sendMessage(int socket, const std::string& message) {
    std::string formattedMessage;
    formattedMessage.push_back(SOH);
    formattedMessage.append(message);
    formattedMessage.push_back(EOT);

    send(socket, formattedMessage.c_str(), formattedMessage.length(), 0);
}

void handleServerCommand(int serverSocket, const std::string& message) {
    // Process commands like HELO, KEEPALIVE, etc.
    std::cout << "Message from server: " << message << std::endl;
    logCommand("Received: " + message);
}

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    if (recv(clientSocket, buffer, sizeof(buffer), 0) > 0) {
        std::string command(buffer);

        if (command[0] == SOH && command.back() == EOT) {
            // Strip SOH and EOT
            command = command.substr(1, command.size() - 2);
            std::vector<std::string> tokens = tokenize(command, ',');

            if (tokens[0] == "HELO") {
                std::string response = "SERVERS,A5_30,127.0.0.1,4030;";
                sendMessage(clientSocket, response);
                logCommand("HELO command received");
            }
            // Add other command handling
        }
        else {
            std::cerr << "Invalid message format received." << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./tsamgroupXX <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);
    int serverSocket = setupServerSocket(port);

    if (serverSocket < 0) {
        return 1;
    }

    fd_set activeSockets;
    FD_ZERO(&activeSockets);
    FD_SET(serverSocket, &activeSockets);

    int maxSocket = serverSocket;

    while (true) {
        fd_set readySockets = activeSockets;

        if (select(maxSocket + 1, &readySockets, nullptr, nullptr, nullptr) < 0) {
            perror("Failed to select");
            continue;
        }

        for (int i = 0; i <= maxSocket; ++i) {
            if (FD_ISSET(i, &readySockets)) {
                if (i == serverSocket) {
                    struct sockaddr_in clientAddr;
                    socklen_t clientAddrSize = sizeof(clientAddr);
                    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
                    if (clientSocket < 0) {
                        perror("Failed to accept client");
                        continue;
                    }

                    FD_SET(clientSocket, &activeSockets);
                    maxSocket = std::max(maxSocket, clientSocket);

                    clients[clientSocket] = inet_ntoa(clientAddr.sin_addr);
                    std::cout << "Client connected: " << clients[clientSocket] << std::endl;
                    logCommand("Client connected: " + clients[clientSocket]);
                }
                else {
                    handleClient(i);
                    close(i);
                    FD_CLR(i, &activeSockets);
                    clients.erase(i);
                }
            }
        }
    }

    close(serverSocket);
    return 0;
}
