#pragma once
#include <fstream>
#include <string>
#include "level.h"

class Logger{
    private:
        Level currentLevel;
        std::ofstream file;

    public:
        void setCurrentLevel(const std::string& newLevel);
        Logger(Level newLevel, const std::string& name);
        ~Logger();
        void logMessage(Level levelMessage, const std::string& text);
        bool isOpen() const { return file.is_open(); }
};