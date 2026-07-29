#include "level.h"

std::string levelToString (Level level) {
    switch(level) {
        case Level::DEBUG: return "DEBUG";
        case Level::INFO:  return "INFO";
        case Level::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

bool stringToLevel (const std::string& str, Level& outLevel) {
    if (str == "DEBUG") {outLevel = Level::DEBUG; return true;}
    if (str == "INFO")  {outLevel = Level::INFO; return true;}
    if (str == "ERROR") {outLevel = Level::ERROR; return true;}
    return false;
}