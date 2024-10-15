// Logger.h

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

class Logger {
private:
    std::ofstream logFile;
    std::mutex mtx;

public:
    Logger(const std::string& filename);
    ~Logger();

    void log(const std::string& tag, const std::string& message);
};

#endif // LOGGER_H
