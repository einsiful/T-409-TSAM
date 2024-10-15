// Einar Árni Bjarnason

#include "tokenizer.h"
#include <sys/socket.h>
#include <map>
#include <string>
#include <vector>

#ifndef CCMD_H
#define CCMD_H

// Struct to hold server information
struct serverInfo
{
    std::string ip;
    int port;
    std::string name;
};

// Struct to hold message information
struct messageInfo
{
    std::string message;
    std::string from;
    std::string to;
};

// Function to log a command
void logCommand(std::string command);

// Function to close a client
void sendToClient(int clientSocket, std::string message);
std::vector<std::vector<std::string>> cmdParser(char *buffer);
std::string uppercase(std::string stringToUpper);
void clientCommand(int clientSocket, fd_set *openSockets, int *maxfds, char *buffer);

#endif // CCMD_H
