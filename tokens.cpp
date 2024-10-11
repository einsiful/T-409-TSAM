#include <vector>
#include <string>
#include <sstream>
#include <algorithm> // For std::count
#include <sys/socket.h>
#include "tokens.h"

// Function to split a string by a given delimiter
std::vector<std::string> tokenizer(const std::string& thingToParse, char splitter) {
    std::vector<std::string> tokens;
    std::string token;

    // Create a stringstream to iterate over the string
    std::stringstream stream(thingToParse);

    // Extract tokens based on the delimiter (splitter)
    while (std::getline(stream, token, splitter)) {
        // If the token contains commas, or if the splitter is a comma, add the token to the list
        if (std::count(token.begin(), token.end(), ',') > 0 || splitter == ',') {
            tokens.push_back(token);
        }
    }

    return tokens;
}
