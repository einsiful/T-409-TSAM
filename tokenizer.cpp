#include <vector>
#include <iostream>
#include <sstream>
#include <sys/socket.h>

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