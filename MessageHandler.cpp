// MessageHandler.cpp

#include "MessageHandler.h"

std::string frameMessage(const std::string &message) {
    std::string framedMessage;
    framedMessage += SOH;
    framedMessage += byteStuff(message);
    framedMessage += EOT;
    return framedMessage;
}

std::string parseFramedMessage(const std::string &data) {
    size_t start = data.find(SOH);
    size_t end = data.find(EOT, start + 1);

    if (start != std::string::npos && end != std::string::npos) {
        std::string message = data.substr(start + 1, end - start - 1);
        return byteUnstuff(message);
    }
    return "";
}

std::string byteStuff(const std::string &message) {
    std::string stuffedMessage;
    for (char c : message) {
        if (c == SOH || c == EOT) {
            stuffedMessage += SOH; // Escape character
        }
        stuffedMessage += c;
    }
    return stuffedMessage;
}

std::string byteUnstuff(const std::string &message) {
    std::string unstuffedMessage;
    bool escape = false;
    for (char c : message) {
        if (escape) {
            unstuffedMessage += c;
            escape = false;
        } else if (c == SOH) {
            escape = true;
        } else {
            unstuffedMessage += c;
        }
    }
    return unstuffedMessage;
}
