// Server.h

#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <map>
#include "Client.h"
#include "SocketHandler.h"
#include "Logger.h"

class Server {
private:
    SocketHandler socketHandler;
    std::map<int, Client*> clients;
    Logger logger;
    int listenSock;
    std::string groupID;
    std::string myIP;
    std::string myPort;

public:
    Server(const std::string& port);
    ~Server();

    void run();

private:
    void acceptNewConnection();
    void handleClientMessage(int clientSock);
    void processCommand(Client* client, const std::string& command);

    // Command handlers
    void handleHELO(Client* client, const std::vector<std::string>& tokens);
    void handleCONNECT(Client* client, const std::vector<std::string>& tokens);
    void handleSERVERS(Client* client);
    void handleKEEPALIVE(Client* client, const std::vector<std::string>& tokens);
    void handleGETMSGS(Client* client, const std::vector<std::string>& tokens);
    void handleSENDMSG(Client* client, const std::vector<std::string>& tokens);
    void handleSTATUSREQ(Client* client);
    void handleSTATUSRESP(Client* client, const std::vector<std::string>& tokens);

    void sendMessage(int sock, const std::string& message);
    std::string receiveMessage(int sock);
};

#endif // SERVER_H
