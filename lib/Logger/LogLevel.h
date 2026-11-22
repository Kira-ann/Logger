#pragma once
#include <cstdint>
#include <string_view>
#include <string>
#include <optional>


enum class LogLevel : std::uint8_t { 
    DEBUG = 0,
    INFO = 1, 
    WARN = 2,
    ERROR = 3 
};

std::string_view toString(LogLevel status);
std::optional<LogLevel> toLevel(const std::string& input_str);