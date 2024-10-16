#include "Server.h"
#include "MessageHandler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <cstring>

struct serverInfo{
    std::string name;
    std::string ip;
    int port;
};

std::map<std::string, serverInfo> knownServers;


Server::Server(const std::string& port): logger("server_log.txt"), groupID("A5_30")
{
    myPort = port;
    myIP = "130.208.240.15";

    listenSock = socketHandler.setupListenSocket(std::stoi(port));
    std::cout << "Server listening on port " << port << std::endl;
    if (listenSock < 0) {
        perror("Failed to setup listening socket");
        exit(EXIT_FAILURE);
    }
}

Server::~Server() {
    for (auto& pair : clients) {
        delete pair.second;
    }
}

void Server::run() {
    fd_set readSockets;
    while (true) {
        readSockets = socketHandler.openSockets;
        int n = select(socketHandler.maxfds + 1, &readSockets, NULL, NULL, NULL);

        if (n < 0) {
            perror("select failed");
            exit(EXIT_FAILURE);
        }

        for (int x = 0; x <= socketHandler.maxfds; ++x) {
            if (FD_ISSET(x, &readSockets)) {
                if (x == listenSock) {
                    acceptNewConnection();
                } else {
                    handleClientMessage(x);
                }
            }
        }
    }
}

void Server::acceptNewConnection() {
    int clientSock;
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    clientSock = accept(listenSock, (struct sockaddr*)&client_addr, &addrlen);

    if (clientSock < 0) {
        perror("Failed to accept new connection");
        return;
    }

    FD_SET(clientSock, &socketHandler.openSockets);
    socketHandler.maxfds = std::max(socketHandler.maxfds, clientSock);

    Client* newClient = new Client(clientSock);
    clients[clientSock] = newClient;

    std::cout << "Accepted new connection on socket " << clientSock << std::endl;

    logger.log("Tag","Accepted new connection on socket " + std::to_string(clientSock));
}

void Server::handleClientMessage(int clientSock) {
    std::string msg = receiveMessage(clientSock);
    if (msg.empty()) {
        std::cout << "Client on socket " << clientSock << " has disconnected" << std::endl;

        socketHandler.closeClient(clientSock);
        delete clients[clientSock];
        clients.erase(clientSock);
        return;
    }

    Client* client = clients[clientSock];
    processCommand(client, msg);
}

void Server::processCommand(Client* client, const std::string& command) {
    logger.log("Tag", "Received command: " + command);

    std::cout << "Processing command from client " << client->sock << ": " << command << std::endl;

    // Tokenize command
    std::vector<std::string> tokens;
    std::stringstream ss(command);
    std::string token;

    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    if (tokens.empty())
        return;

    if (tokens[0] == "HELO") {
        handleHELO(client, tokens);
    } else if (tokens[0] == "SERVERS") {
        handleSERVERS(client, tokens);
    } else if (tokens[0] == "CONNECT") {
        handleCONNECT(client, tokens);
    } else if (tokens[0] == "KEEPALIVE") {
        handleKEEPALIVE(client, tokens);
    } else if (tokens[0] == "GETMSGS") {
        handleGETMSGS(client, tokens);
    } else if (tokens[0] == "SENDMSG") {
        handleSENDMSG(client, tokens);
    } else if (tokens[0] == "STATUSREQ") {
        handleSTATUSREQ(client);
    } else if (tokens[0] == "STATUSRESP") {
        handleSTATUSRESP(client, tokens);
    } else {
        logger.log("Tag", "Unknown command: " + tokens[0]);
    }
}

void Server::handleHELO(Client* client, const std::vector<std::string>& tokens) {
    if (tokens.size() != 2) {
        logger.log("Tag", "Invalid HELO command");
        return;
    }

    client->name = tokens[1];
    client->isServer = true;

    // Log the received HELO
    logger.log("Tag", "Processed HELO from " + client->name);

    // Send SERVERS response
    sendSERVERS(client);
}

void Server::handleCONNECT(Client* client, const std::vector<std::string>& tokens) {
    if (tokens.size() != 3) {
        logger.log("Tag", "Invalid CONNECT command");
        return;
    }

    logger.log("Tag", "Attempting to connect to " + tokens[1] + ":" + tokens[2]);

    // Attempt to set up a client socket and connect to the server
    int newSocket = socketHandler.setupClientSocket(tokens[1], std::stoi(tokens[2]));

    if (newSocket != -1) {
        // Successfully connected, create a new Client object
        Client* newServerClient = new Client(newSocket);
        newServerClient->isServer = true;
        clients[newSocket] = newServerClient;

        // Send HELO to the newly connected server
        std::string heloMessage = "HELO," + groupID;
        sendMessage(newSocket, heloMessage);
        logger.log("Tag", "Sent HELO to " + tokens[1] + ":" + tokens[2]);

    } else {
        logger.log("Tag", "Failed to connect to " + tokens[1] + ":" + tokens[2]);
    }
}

// Handles GETMSGS command
void Server::handleGETMSGS(Client* client, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        logger.log("Error", "Invalid GETMSGS command received.");
        return;
    }

    std::string groupID = tokens[1];

    // Handle retrieving messages for the specified group (client's group ID)
    // Placeholder: Implement message retrieval from storage
    std::string response = "No messages available for group " + groupID;
    sendMessage(client->sock, response);

    logger.log("Tag", "Processed GETMSGS for group " + groupID);
}

// Handles SENDMSG command
void Server::handleSENDMSG(Client* client, const std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        logger.log("Tag", "Invalid SENDMSG command received.");
        return;
    }

    std::string toGroupID = tokens[1];
    std::string message = tokens[2];

    // Handle sending message to another group
    // Placeholder: Implement message forwarding or storage
    logger.log("Tag", "Processed SENDMSG to group " + toGroupID + ": " + message);
}

// Handles SERVERS command
void Server::handleSERVERS(Client* client, const std::vector<std::string>& tokens) {
    // Get server list and add to client's known servers
    // the format of the server string is as follows: SERVERS,A5_1,130.208.243.61,8888;A5_2,10.2.132.12,10042;
    // split on every semicolon, and then comma, but exclude the SERVERS that comes first
    std::string serverList = tokens[1];

    for (int i = 1; i < tokens.size(); i++) {
        std::cout << "Server token: " << tokens[i] << std::endl;
    }
    
    std::cout << "Received server list from " << client->name << std::endl;
    std::cout << "Servers: " << serverList << std::endl;
    

}

// Handles KEEPALIVE command
void Server::handleKEEPALIVE(Client* client, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        logger.log("Tag", "Invalid KEEPALIVE command received.");
        return;
    }

    // Handle the keepalive message
    logger.log("Tag", "Received KEEPALIVE from client " + client->name);
}

// Handles STATUSREQ command
void Server::handleSTATUSREQ(Client* client) {
    // Respond with status of messages being held for other servers
    // Placeholder: Implement status response
    std::string response = "STATUSRESP,A5_30,10;";
    sendMessage(client->sock, response);

    logger.log("Tag", "Processed STATUSREQ command");
}

// Handles STATUSRESP command
void Server::handleSTATUSRESP(Client* client, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        logger.log("Tag", "Invalid STATUSRESP command received.");
        return;
    }

    // Handle the status response message
    logger.log("Tag", "Received STATUSRESP from client " + client->name);
}

// Sends a HELO message to the client
void Server::sendHELO(Client* client) {
    std::string heloMessage = "HELO," + groupID;
    sendMessage(client->sock, heloMessage);
}

// Sends a SERVERS message to the client
void Server::sendSERVERS(Client* client) {
    // Placeholder: Implement server list generation
    // std::string response = "SERVERS," + groupID + ",
    //
    std::string response = generateServerList(clients, socketHandler);
    sendMessage(client->sock, response);
}

void Server::sendMessage(int sock, const std::string& message) {
    std::string framedMessage = frameMessage(message);  // Assuming you frame the message properly
    ssize_t bytesSent = send(sock, framedMessage.c_str(), framedMessage.length(), 0);

    if (bytesSent < 0) {
        logger.log("Tag", "Failed to send message on socket " + std::to_string(sock) + ": " + strerror(errno));
        perror("send failed");
    } else {
        logger.log("Tag", "Sent " + std::to_string(bytesSent) + " bytes on socket " + std::to_string(sock));
    }
}

std::string Server::receiveMessage(int sock) {
    char buffer[1024];
    ssize_t bytesRead = recv(sock, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        return "";
    }
    buffer[bytesRead] = '\0';
    return parseFramedMessage(std::string(buffer));
}

std::string Server::generateServerList(std::map<int, Client *> clients, SocketHandler &socketHandler) {
    // Generates the response to QUERYSERVERS i.e. SERVERS
    std::string retStr;
    retStr.append("\2SERVERS,"+ groupID + ',' + myIP + ',' + myPort + ';');
    for (auto &pair: clients) {
        std::string serverName = pair.second->name;
        if (knownServers[serverName].name != "") {
            retStr.append(knownServers[serverName].name);
            retStr.append(",");
            retStr.append(knownServers[serverName].ip);
            retStr.append(",");
            retStr.append(std::to_string(knownServers[serverName].port));
            retStr.append(";");
        }
    }
    retStr.append("\3");
    return retStr;
}