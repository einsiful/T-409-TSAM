// SocketHandler.h

#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <string>
#include <map>
#include <sys/select.h>
#include "Client.h"
#include "Logger.h"

class SocketHandler {
public:
    fd_set openSockets;
    int maxfds;
    Logger logger;

    SocketHandler();
    ~SocketHandler();

    int setupListenSocket(int portno);
    int setupClientSocket(const std::string& ip, int portno);
    void closeClient(int clientSocket);
};

#endif // SOCKETHANDLER_H
