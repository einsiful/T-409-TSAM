// SocketHandler.cpp

#include "SocketHandler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <algorithm>

SocketHandler::SocketHandler(): logger("socket_log.txt")
{
    FD_ZERO(&openSockets);
    maxfds = 0;
}

SocketHandler::~SocketHandler() {
    // Close all sockets on destruction
}

int SocketHandler::setupListenSocket(int portno) {
    int listenSock;
    struct sockaddr_in sk_addr;
    int set = 1;

    // Create socket
    if ((listenSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to open socket");
        return -1;
    }

    // Set socket options
    if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set)) < 0) {
        perror("Failed to set SO_REUSEADDR");
    }

    // Bind to address
    memset(&sk_addr, 0, sizeof(sk_addr));
    sk_addr.sin_family = AF_INET;
    sk_addr.sin_addr.s_addr = INADDR_ANY;
    sk_addr.sin_port = htons(portno);

    if (bind(listenSock, (struct sockaddr *)&sk_addr, sizeof(sk_addr)) < 0) {
        perror("Failed to bind socket");
        close(listenSock);
        return -1;
    }

    // Listen
    if (listen(listenSock, 5) < 0) {
        perror("Failed to listen on socket");
        close(listenSock);
        return -1;
    }

    // Add to open sockets
    FD_SET(listenSock, &openSockets);
    maxfds = std::max(maxfds, listenSock);

    return listenSock;
}

int SocketHandler::setupClientSocket(const std::string& ip, int portno) {
    int clientSock;
    struct sockaddr_in server_addr;

    // Create the socket
    if ((clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to open socket");
        return -1;
    }

    // Set the socket to blocking mode
    int flags = fcntl(clientSock, F_GETFL, 0);
    fcntl(clientSock, F_SETFL, flags & ~O_NONBLOCK);

    // Set up the server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    inet_aton(ip.c_str(), &server_addr.sin_addr);

    // Attempt to connect to the server
    int connectResult = connect(clientSock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (connectResult < 0) {
        perror("Failed to connect to server");
        close(clientSock);
        return -1;
    }

    // Log successful connection
    logger.log("Tag", "Successfully connected to " + ip + ":" + std::to_string(portno));

    // Add the socket to the set of open sockets for monitoring
    FD_SET(clientSock, &openSockets);
    maxfds = std::max(maxfds, clientSock);

    return clientSock;
}


void SocketHandler::closeClient(int clientSocket) {
    printf("Closing client socket %d\n", clientSocket);
    close(clientSocket);
    FD_CLR(clientSocket, &openSockets);

    if (maxfds == clientSocket) {
        maxfds = 0;
        for (int fd = 0; fd <= FD_SETSIZE; ++fd) {
            if (FD_ISSET(fd, &openSockets)) {
                maxfds = std::max(maxfds, fd);
            }
        }
    }

}
