#include "LogLevel.h"

std::string_view toString(LogLevel status){
    switch(status){
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default : return "UNKNOWN";
    }
}

std::optional<LogLevel> toLevel(const std::string& input_str){
    if (input_str == "DEBUG") return LogLevel::DEBUG;
    if (input_str == "INFO") return LogLevel::INFO;
    if (input_str == "WARN") return LogLevel::WARN;
    if (input_str == "ERROR") return LogLevel::ERROR;
    return std::nullopt;
}