// Client.h

#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <ctime>
#include <unistd.h>

class Client {
public:
    int sock;
    std::string name;
    time_t timestamp;
    bool isServer; // True if this client is another server

    Client(int socket);
    ~Client();
};

#endif // CLIENT_H
