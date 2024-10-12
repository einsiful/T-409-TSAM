//
// Simple chat server for TSAM-409
//
// Command line: ./chat_server 4000 
//
// Author: Jacky Mallett (jacky@ru.is)
//
#include <fstream>
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
#include <map>

#include <unistd.h>

#include "tokenizer.h"


// fix SOCK_NONBLOCK for OSX
#ifndef SOCK_NONBLOCK
#include <fcntl.h>
#define SOCK_NONBLOCK O_NONBLOCK
#endif

#define BACKLOG  5          // Allowed length of queue of waiting connections
#define BUFFER_SIZE 1024
char SOH = 0x01;  // Start of Header (SOH)
char EOT = 0x04;  // End of Transmission (EOT)

// Simple class for handling connections from clients.
//
// Client(int socket) - socket to send/receive traffic from client.
class Client
{
  public:
    int sock;              // socket of client connection
    std::string name;           // Limit length of name of client's user

    Client(int socket) : sock(socket){} 

    ~Client(){}            // Virtual destructor defined for base class
};

// Note: map is not necessarily the most efficient method to use here,
// especially for a server with large numbers of simulataneous connections,
// where performance is also expected to be an issue.
//
// Quite often a simple array can be used as a lookup table, 
// (indexed on socket no.) sacrificing memory for speed.

std::map<int, Client*> clients; // Lookup table for per Client information
std::map<std::string, std::vector<std::string>> messageCache;

// Open socket for specified port.
//
// Returns -1 if unable to create the socket for any reason.

// Logging function
void logCommand(const std::string& logMessage) {
    std::ofstream logFile("server.log", std::ios::app);
    time_t now = time(0);
    logFile << "Timestamp: " << ctime(&now) << "Command: " << logMessage << std::endl;
    logFile.close();
}

int open_socket(int portno)
{
   struct sockaddr_in sk_addr;   // address settings for bind()
   int sock;                     // socket opened for this port
   int set = 1;                  // for setsockopt

   // Create socket for connection. Set to be non-blocking, so recv will
   // return immediately if there isn't anything waiting to be read.
#ifdef __APPLE__     
   if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("Failed to open socket");
      return(-1);
   }
#else
   if((sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0)
   {
     perror("Failed to open socket");
    return(-1);
   }
#endif

   // Turn on SO_REUSEADDR to allow socket to be quickly reused after 
   // program exit.

   if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set)) < 0)
   {
      perror("Failed to set SO_REUSEADDR:");
   }
   set = 1;
#ifdef __APPLE__     
   if(setsockopt(sock, SOL_SOCKET, SOCK_NONBLOCK, &set, sizeof(set)) < 0)
   {
     perror("Failed to set SOCK_NOBBLOCK");
   }
#endif
   memset(&sk_addr, 0, sizeof(sk_addr));

   sk_addr.sin_family      = AF_INET;
   sk_addr.sin_addr.s_addr = INADDR_ANY;
   sk_addr.sin_port        = htons(portno);

   // Bind to socket to listen for connections from clients

   if(bind(sock, (struct sockaddr *)&sk_addr, sizeof(sk_addr)) < 0)
   {
      perror("Failed to bind to socket:");
      return(-1);
   }
   else
   {
      return(sock);
   }
}

// Close a client's connection, remove it from the client list, and
// tidy up select sockets afterwards.

void closeClient(int clientSocket, fd_set *openSockets, int *maxfds)
{

     printf("Client closed connection: %d\n", clientSocket);

     // If this client's socket is maxfds then the next lowest
     // one has to be determined. Socket fd's can be reused by the Kernel,
     // so there aren't any nice ways to do this.

     close(clientSocket);      

     if(*maxfds == clientSocket)
     {
        for(auto const& p : clients)
        {
            *maxfds = std::max(*maxfds, p.second->sock);
        }
     }

     // And remove from the list of open sockets.

     FD_CLR(clientSocket, openSockets);

}

std::string uppercase(std::string stringToUpper)
{
    std::transform(stringToUpper.begin(), stringToUpper.end(), stringToUpper.begin(), ::toupper);
    return stringToUpper;
}

std::map<std::string, std::vector<std::string>> fetch_messages(std::string user)
{
    std::map<std::string, std::vector<std::string>> messages;
    for(auto const& pair : messageCache)
    {
        if(pair.first == user)
        {
            messages[user] = pair.second;
        }
    }
    return messages;
}

// Process command from client on the server

void clientCommand(int clientSocket, fd_set *openSockets, int *maxfds, char *buffer) 
{
    std::string command(buffer);
    std::vector<std::string> tokens = tokenizer(command, ',');

    // Check if the CONNECT command is received with the right number of arguments
    if ((tokens[0].compare("CONNECT") == 0) && (tokens.size() == 3)) 
    {
        std::string serverIp = tokens[1];   // IP of the server to connect to
        int serverPort = std::stoi(tokens[2]);  // Convert port to integer

        // Create a new socket to connect to the given server
        int connectSock = socket(AF_INET, SOCK_STREAM, 0);
        if (connectSock < 0) {
            perror("Failed to create socket");
            logCommand("CONNECT failed to create socket");
            return;
        }

        // Setup server address struct
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);

        // Convert IP address to binary form
        if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0) {
            perror("Invalid IP address");
            logCommand("CONNECT invalid IP address: " + serverIp);
            close(connectSock);
            return;
        }

        // Attempt to connect to the server
        if (connect(connectSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            perror("Failed to connect to server");
            logCommand("CONNECT failed to connect to server: " + serverIp + ":" + std::to_string(serverPort));
            close(connectSock);
            return;
        }

        // Log successful connection
        logCommand("Successfully connected to " + serverIp + ":" + std::to_string(serverPort));
        
        // Send HELO with SOH and EOT
        std::string soh(1, SOH);
        std::string eot(1, EOT);
        std::string heloMsg = soh + "HELO,A5_30" + eot;
        send(connectSock, heloMsg.c_str(), heloMsg.size(), 0);

        // Receive response from the server and process it
        char recvBuffer[BUFFER_SIZE];
        int received = recv(connectSock, recvBuffer, sizeof(recvBuffer), 0);
        if (received > 0) {
            std::string receivedMsg(recvBuffer, received);
            std::vector<std::string> responseTokens = tokenizer(receivedMsg, ',');
        }

        close(connectSock);  // Close after message processing
    }
    else {
        // Handle failed connections or unknown commands
        char response[256] = "Unknown or failed connection command.\n";
        send(clientSocket, response, sizeof(response), 0);
        logCommand("CONNECT command failed or was unknown.");
    }
}


int main(int argc, char* argv[])
{
    bool finished;
    int listenSock;                 // Socket for connections to server
    int clientSock;                 // Socket of connecting client
    fd_set openSockets;             // Current open sockets 
    fd_set readSockets;             // Socket list for select()        
    fd_set exceptSockets;           // Exception socket list
    int maxfds;                     // Passed to select() as max fd in set
    struct sockaddr_in client;
    socklen_t clientLen;
    char buffer[1025];              // buffer for reading from clients

    if(argc != 2)
    {
        printf("Usage: chat_server <ip port>\n");
        exit(0);
    }

    // Setup socket for server to listen to

    listenSock = open_socket(atoi(argv[1])); 
    printf("Listening on port: %d\n", atoi(argv[1]));

    if(listen(listenSock, BACKLOG) < 0)
    {
        printf("Listen failed on port %s\n", argv[1]);
        exit(0);
    }
    else 
    // Add listen socket to socket set we are monitoring
    {
        FD_ZERO(&openSockets);
        FD_SET(listenSock, &openSockets);
        maxfds = listenSock;
    }

    finished = false;

    while(!finished)
    {
        // Get modifiable copy of readSockets
        readSockets = exceptSockets = openSockets;
        memset(buffer, 0, sizeof(buffer));

        // Look at sockets and see which ones have something to be read()
        int n = select(maxfds + 1, &readSockets, NULL, &exceptSockets, NULL);

        if(n < 0)
        {
            perror("select failed - closing down\n");
            finished = true;
        }
        else
        {
            // First, accept  any new connections to the server on the listening socket
            if(FD_ISSET(listenSock, &readSockets))
            {
               clientSock = accept(listenSock, (struct sockaddr *)&client,
                                   &clientLen);
               printf("accept***\n");
               // Add new client to the list of open sockets
               FD_SET(clientSock, &openSockets);

               // And update the maximum file descriptor
               maxfds = std::max(maxfds, clientSock) ;

               // create a new client to store information.
               clients[clientSock] = new Client(clientSock);

               // Decrement the number of sockets waiting to be dealt with
               n--;

               printf("Client connected on server: %d\n", clientSock);
            }
            // Now check for commands from clients
            std::list<Client *> disconnectedClients;  
            while(n-- > 0)
            {
               for(auto const& pair : clients)
               {
                  Client *client = pair.second;

                  if(FD_ISSET(client->sock, &readSockets))
                  {
                      // recv() == 0 means client has closed connection
                      if(recv(client->sock, buffer, sizeof(buffer), MSG_DONTWAIT) == 0)
                      {
                          disconnectedClients.push_back(client);
                          closeClient(client->sock, &openSockets, &maxfds);

                      }
                      // We don't check for -1 (nothing received) because select()
                      // only triggers if there is something on the socket for us.
                      else
                      {
                          std::cout << buffer << std::endl;
                          clientCommand(client->sock, &openSockets, &maxfds, buffer);
                      }
                  }
               }
               // Remove client from the clients list
               for(auto const& c : disconnectedClients)
                  clients.erase(c->sock);
            }
        }
    }
}
