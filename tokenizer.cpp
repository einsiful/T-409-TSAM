#include <vector>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <algorithm>

std::vector<std::string> tokenizer(std::string thingToParse, char splitter){

    std::vector<std::string> tokens;
    std::string token;

    // Split command from client into tokens for parsing
    std::stringstream stream(thingToParse);

    while(std::getline(stream, token, splitter)){
        if (std::count(token.begin(), token.end(), ',') > 0 || splitter == ',') {
            tokens.push_back(token);
            //std::cout << token << std::endl;
        }
    }
    return tokens;
}

// TODO: LAGA!!!
std::vector<std::string> messageSeperator(char *buffer, int sock){
    std::vector<std::string> commandList;
    std::string command;
    std::string bufferStr = buffer;


    int countSOH;
    int countEOT;
    bool keepRunning = true;
    int count;
    while (keepRunning){
        for (char &character : bufferStr){
            if (character == (char)4){
                commandList.push_back(command);
                countEOT++;
            } else if (character == (char)1){
                command = "";
                countSOH++;
            } else {
                command.push_back(character);
            }
        }
        count++;

        if (countSOH == countEOT || count > 4){
            keepRunning = false;
        } else {
            char newBuff[5000] = {0};
            recv(sock, &newBuff, sizeof(newBuff), MSG_DONTWAIT);
            bufferStr = newBuff;
        }
    }
    return commandList;
}