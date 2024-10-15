// Client.cpp

#include "Client.h"

Client::Client(int socket) : sock(socket), name(""), timestamp(time(nullptr)), isServer(false) {}

Client::~Client() {
    // Close the socket when the client object is destroyed
    close(sock);
}
