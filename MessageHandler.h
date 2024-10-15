// MessageHandler.h

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <string>

const char SOH = 0x01; // Start of Header
const char EOT = 0x04; // End of Transmission

std::string frameMessage(const std::string &message);
std::string parseFramedMessage(const std::string &data);
std::string byteStuff(const std::string &message);
std::string byteUnstuff(const std::string &message);

#endif // MESSAGEHANDLER_H
