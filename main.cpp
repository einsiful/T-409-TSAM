// main.cpp

#include "Server.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: ./tsamgroup<YourGroupID> <port>" << std::endl;
        return 1;
    }

    Server server(argv[1]);
    server.run();

    return 0;
}
