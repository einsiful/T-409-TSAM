#include <vector>
#include <iostream>
#include <sstream>
#include <sys/socket.h>

#ifndef TSAM_A5_30_TOKENIZER_H
#define TSAM_A5_30_TOKENIZER_H


std::vector<std::string> tokenizer(std::string thingToParse, char splitter);
std::vector<std::string> messageSeperator(char *buffer, int sock);


#endif //TSAM_A5_30_TOKENIZER_H
