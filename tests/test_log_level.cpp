#include <gtest/gtest.h>

#include <array>
#include <string>

#include "LogLevel.h"

TEST(LogLevelTest, ToStringReturnsReadableNames) {
    EXPECT_EQ(toString(LogLevel::DEBUG), "DEBUG");
    EXPECT_EQ(toString(LogLevel::INFO),  "INFO");
    EXPECT_EQ(toString(LogLevel::WARN),  "WARN");
    EXPECT_EQ(toString(LogLevel::ERROR), "ERROR");
}

TEST(LogLevelTest, ToStringReturnsUnknownForInvalidValue) {
    EXPECT_EQ(toString(static_cast<LogLevel>(255)), "UNKNOWN");
}

TEST(LogLevelTest, ToLevelParsesValidNames) {
    EXPECT_EQ(toLevel("DEBUG"), LogLevel::DEBUG);
    EXPECT_EQ(toLevel("INFO"),  LogLevel::INFO);
    EXPECT_EQ(toLevel("WARN"),  LogLevel::WARN);
    EXPECT_EQ(toLevel("ERROR"), LogLevel::ERROR);
}

TEST(LogLevelTest, ToLevelRejectsInvalidNames) {
    for (const char* input : {"", "debug", "Debug", "WARNING", "TRACE", " INFO", "INFO "}) {
        EXPECT_EQ(toLevel(input), std::nullopt) << "input: '" << input << "'";
    }
}

TEST(LogLevelTest, RoundTrip) {
    constexpr std::array levels{
        LogLevel::DEBUG, LogLevel::INFO, LogLevel::WARN, LogLevel::ERROR
    };
    for (auto level : levels) {
        EXPECT_EQ(toLevel(std::string(toString(level))), level);
    }
}

TEST(LogLevelTest, LevelsAreOrderedBySeverity) {
    EXPECT_LT(LogLevel::DEBUG, LogLevel::INFO);
    EXPECT_LT(LogLevel::INFO,  LogLevel::WARN);
    EXPECT_LT(LogLevel::WARN,  LogLevel::ERROR);
}