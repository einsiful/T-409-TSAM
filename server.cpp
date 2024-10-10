//
// Simple chat server with additional commands for TSAM-409
//
// Command line: ./chat_server 4000
//
// Author: Jacky Mallett (jacky@ru.is) - Extended by OpenAI
//

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <algorithm>
#include <map>
#include <vector>
#include <list>
#include <iostream>
#include <sstream>
#include <thread>
#include <fstream>
#include <ctime>
#include <fcntl.h> // For fcntl()

std::string groupId = "30";
std::string serverGroupId = "A5_" + groupId;

#define BACKLOG 5  // Allowed length of queue of waiting connections

class Client {
  public:
    int sock;              // socket of client connection
    std::string name;      // Name of the connected client (group ID)

    Client(int socket) : sock(socket) {}

    ~Client() {}  // Destructor for cleanup
};

// Maps to store clients and message cache
std::map<int, Client*> clients;
std::map<std::string, std::vector<std::string>> messageCache;

// Logging function
void logCommand(const std::string& logMessage) {
    std::ofstream logFile("server.log", std::ios::app);
    time_t now = time(0);
    logFile << "Timestamp: " << ctime(&now) << "Command: " << logMessage << std::endl;
    logFile.close();
}

// Open socket for specified port
int open_socket(int portno) {
    struct sockaddr_in sk_addr;
    int sock;
    int set = 1;

    if ((sock = socket(AF_INET, SOCK_STREAM | SOCK_STREAM, 0)) < 0) {
        perror("Failed to open socket");
        return (-1);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set)) < 0) {
        perror("Failed to set SO_REUSEADDR");
    }

    memset(&sk_addr, 0, sizeof(sk_addr));
    sk_addr.sin_family = AF_INET;
    sk_addr.sin_addr.s_addr = INADDR_ANY;
    sk_addr.sin_port = htons(portno);

    if (bind(sock, (struct sockaddr*)&sk_addr, sizeof(sk_addr)) < 0) {
        perror("Failed to bind to socket");
        return (-1);
    } else {
        return (sock);
    }
}

// Close a client's connection, remove it from the client list
void closeClient(int clientSocket, fd_set* openSockets, int* maxfds) {
    printf("Client closed connection: %d\n", clientSocket);
    close(clientSocket);

    if (*maxfds == clientSocket) {
        for (auto const& p : clients) {
            *maxfds = std::max(*maxfds, p.second->sock);
        }
    }

    FD_CLR(clientSocket, openSockets);
    clients.erase(clientSocket);
}

// Helper function to convert strings to uppercase
std::string uppercase(std::string stringToUpper) {
    std::transform(stringToUpper.begin(), stringToUpper.end(), stringToUpper.begin(), ::toupper);
    return stringToUpper;
}

// Process command from client
void clientCommand(int clientSocket, fd_set* openSockets, int* maxfds, char* buffer) {
    std::vector<std::string> tokens;
    std::string token;

    std::stringstream stream(buffer);
    while (stream >> token) tokens.push_back(token);

    if ((tokens[0].compare("CONNECT") == 0) && (tokens.size() == 2)) {
        clients[clientSocket]->name = tokens[1];
        std::cout << "Client connected: " << tokens[1] << std::endl;
        logCommand("CONNECT from " + tokens[1]);
    }
    else if (tokens[0].compare("LEAVE") == 0) {
        closeClient(clientSocket, openSockets, maxfds);
        logCommand("LEAVE from client: " + std::to_string(clientSocket));
    }
    else if (tokens[0].compare("WHO") == 0) {
        std::string msg;
        for (auto const& names : clients) {
            msg += names.second->name + ",";
        }
        send(clientSocket, msg.c_str(), msg.length() - 1, 0);
        logCommand("WHO request");
    }
    else if (tokens[0].compare("HELO") == 0 && tokens.size() == 2) {
        std::string response = "SERVERS," + serverGroupId + "," + "127.0.0.1," + std::to_string(clientSocket) + ";";
        for (const auto& client : clients) {
            if (client.second->name != "") {
                response += client.second->name + ",127.0.0.1," + std::to_string(client.second->sock) + ";";
            }
        }
        send(clientSocket, response.c_str(), response.length(), 0);
        std::cout << "HELO command from " << tokens[1] << " Responded with: " << response << std::endl;
        logCommand("HELO from " + tokens[1]);
    }
    else if (tokens[0].compare("KEEPALIVE") == 0 && tokens.size() == 2) {
        int messageCount = atoi(tokens[1].c_str());
        std::cout << "KEEPALIVE: " << messageCount << " messages waiting." << std::endl;
        logCommand("KEEPALIVE from " + clients[clientSocket]->name);
    }
    else if (tokens[0].compare("GETMSGS") == 0 && tokens.size() == 2) {
        std::string groupId = tokens[1];
        std::cout << "GETMSGS request for group: " << groupId << std::endl;
        std::string messages = "Messages for " + groupId;
        send(clientSocket, messages.c_str(), messages.length(), 0);
        logCommand("GETMSGS from " + groupId);
    }
    else if (tokens[0].compare("SENDMSG") == 0 && tokens.size() >= 4) {
        std::string toGroup = tokens[1];
        std::string fromGroup = tokens[2];
        std::string messageContent;
        for (int i = 3; i < tokens.size(); i++) {
            messageContent += tokens[i] + " ";
        }
        std::cout << "SENDMSG: From " << fromGroup << " To " << toGroup << " Message: " << messageContent << std::endl;
        messageCache[toGroup].push_back(messageContent);
        logCommand("SENDMSG from " + fromGroup + " to " + toGroup);
    }
    else if (tokens[0].compare("STATUSREQ") == 0) {
        std::string statusResponse = "STATUSRESP," + serverGroupId;
        for (const auto& cache : messageCache) {
            statusResponse += "," + cache.first + "," + std::to_string(cache.second.size());
        }
        send(clientSocket, statusResponse.c_str(), statusResponse.length(), 0);
        logCommand("STATUSREQ");
    }
    else if (tokens[0].compare("STATUSRESP") == 0 && tokens.size() >= 3) {
        std::cout << "STATUSRESP from server: " << tokens[1] << " with messages: " << tokens[2] << std::endl;
        logCommand("STATUSRESP");
    }
    else {
        std::cout << "Unknown command from client: " << buffer << std::endl;
        logCommand("Unknown command: " + std::string(buffer));
    }
}

// Main function
int main(int argc, char* argv[]) {
    bool finished;
    int listenSock;
    int clientSock;
    fd_set openSockets, readSockets, exceptSockets;
    int maxfds;
    struct sockaddr_in client;
    socklen_t clientLen;
    char buffer[1025];

    if (argc != 2) {
        printf("Usage: chat_server <ip port>\n");
        exit(0);
    }

    listenSock = open_socket(atoi(argv[1]));
    printf("Listening on port: %d\n", atoi(argv[1]));

    if (listen(listenSock, BACKLOG) < 0) {
        printf("Listen failed on port %s\n", argv[1]);
        exit(0);
    }

    FD_ZERO(&openSockets);
    FD_SET(listenSock, &openSockets);
    maxfds = listenSock;

    finished = false;

    while (!finished) {
        readSockets = exceptSockets = openSockets;
        memset(buffer, 0, sizeof(buffer));

        int n = select(maxfds + 1, &readSockets, NULL, &exceptSockets, NULL);

        if (n < 0) {
            perror("select failed - closing down\n");
            finished = true;
        } else {
            if (FD_ISSET(listenSock, &readSockets)) {
                clientSock = accept(listenSock, (struct sockaddr*)&client, &clientLen);
                printf("accept***\n");
                FD_SET(clientSock, &openSockets);
                maxfds = std::max(maxfds, clientSock);
                clients[clientSock] = new Client(clientSock);
                printf("Client connected on server: %d\n", clientSock);
                n--;
            }

            std::list<Client*> disconnectedClients;
            while (n-- > 0) {
                for (auto const& pair : clients) {
                    Client* client = pair.second;
                    if (FD_ISSET(client->sock, &readSockets)) {
                        if (recv(client->sock, buffer, sizeof(buffer), MSG_DONTWAIT) == 0) {
                            disconnectedClients.push_back(client);
                            closeClient(client->sock, &openSockets, &maxfds);
                        } else {
                            std::cout << buffer << std::endl;
                            clientCommand(client->sock, &openSockets, &maxfds, buffer);
                        }
                    }
                }
                for (auto const& c : disconnectedClients)
                    clients.erase(c->sock);
            }
        }
    }
}
