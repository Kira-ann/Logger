#define _POSIX_C_SOURCE 200809L

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include "level.h"
#include "logger.h"

Logger::Logger(Level newLevel, const std::string& name){
    file.open(name, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << name << std::endl;
    }
    currentLevel = newLevel;
}

Logger::~Logger(){
    if(file.is_open()){
        file.close();
    }
}

void Logger::setCurrentLevel(const std::string& newLevel){
    Level level;
    if (stringToLevel(newLevel, level)){
        currentLevel = level;
    }
}

void Logger::logMessage(Level levelMessage, const std::string& text){
    if (!file.is_open() || levelMessage < currentLevel) {
        return;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
    if (localtime_r(&in_time_t, &tm) == nullptr) {
        file << "Unknown time" << ' ' << levelToString(levelMessage) << ' ' << text << '\n';
        return;
    }

    file << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << ' ' << levelToString(levelMessage) << ' ' << text << '\n';
}