// Logger.cpp

#include "Logger.h"
#include <ctime>

Logger::Logger(const std::string& filename) {
    logFile.open(filename, std::ios::app);
}

Logger::~Logger() {
    if (logFile.is_open())
        logFile.close();
}

void Logger::log(const std::string& tag, const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);

    std::time_t now = std::time(nullptr);
    std::string timeStr = std::ctime(&now);
    timeStr.erase(timeStr.find_last_not_of(" \n\r\t") + 1);

    logFile << "[" << timeStr << "] " << tag << ": " << message << std::endl;
}
