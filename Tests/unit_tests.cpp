#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include "logger.h"
#include "level.h"


void testLevelOrder() {
    assert(static_cast<int>(Level::DEBUG) == 0);
    assert(static_cast<int>(Level::INFO) == 1);
    assert(static_cast<int>(Level::ERROR) == 2);
    assert(Level::DEBUG < Level::INFO);
    assert(Level::INFO < Level::ERROR);
}

void testStringToLevelCaseSensitive() {
    Level out;
    assert(!stringToLevel("debug", out));  // должно быть только "DEBUG"
    assert(!stringToLevel("Info", out));   // только "INFO"
    assert(!stringToLevel("error", out));  // только "ERROR"
    assert(stringToLevel("DEBUG", out) && out == Level::DEBUG);
    assert(stringToLevel("INFO", out) && out == Level::INFO);
    assert(stringToLevel("ERROR", out) && out == Level::ERROR);
}

void testLoggerWrite() {
    const std::string testFile = "test_logger.log";
    {
        Logger logger(Level::INFO, testFile);
        logger.logMessage(Level::INFO, "Test message");
    }

    std::ifstream file(testFile);
    std::string line;
    std::getline(file, line);
    file.close();

    assert(line.find("INFO") != std::string::npos);
    assert(line.find("Test message") != std::string::npos);

    std::remove(testFile.c_str());
}

void testLoggerFilter() {
    const std::string testFile = "test_filter.log";
    {
        Logger logger(Level::ERROR, testFile);
        logger.logMessage(Level::DEBUG, "Debug message");
        logger.logMessage(Level::INFO, "Info message");
        logger.logMessage(Level::ERROR, "Error message");
    }

    std::ifstream file(testFile);
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line;
    }
    file.close();

    assert(content.find("Debug message") == std::string::npos);
    assert(content.find("Info message") == std::string::npos);
    assert(content.find("Error message") != std::string::npos);

    std::remove(testFile.c_str());
}

int main() {
    assert(levelToString(Level::DEBUG) == "DEBUG");
    assert(levelToString(Level::INFO)  == "INFO");
    assert(levelToString(Level::ERROR) == "ERROR");

    Level out;
    assert(stringToLevel("DEBUG", out) && out == Level::DEBUG);
    assert(stringToLevel("INFO",  out) && out == Level::INFO);
    assert(stringToLevel("ERROR", out) && out == Level::ERROR);
    assert(!stringToLevel("BANANA", out));
    assert(!stringToLevel("", out));

    testLevelOrder();
    testStringToLevelCaseSensitive();
    testLoggerWrite();
    testLoggerFilter();

}