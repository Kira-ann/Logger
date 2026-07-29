#pragma once
#include <string>

enum class Level {
    DEBUG,
    INFO,
    ERROR
};

std::string levelToString(Level level);
bool stringToLevel (const std::string& str, Level& outLevel);