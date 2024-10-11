// Description: This file contains the declaration of the split function
#ifndef TOKENS_H
#define TOKENS_H

#include <vector>
#include <iostream>
#include <sstream>
#include <sys/socket.h>

// Declaration of the split function
std::vector<std::string> tokenizer(const std::string toParse, char splitter);
std::vector<std::string> messageSeparator(char *buffer, int sock);

#endif  // TOKENS_H
