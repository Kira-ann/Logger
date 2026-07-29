#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <string>
#include "level.h"
#include "logger.h"

struct LogEntry {
    std::string message;
    Level level;
};

std::queue<LogEntry> logQueue;
std::mutex queueMutex;
std::condition_variable cv;
std::atomic<bool> stopFlag(false);

void workerThread(Logger& logger) {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex); // блокируем очередь для другого потока
        cv.wait(lock, [&] { return !logQueue.empty() || stopFlag.load(); });

        if (logQueue.empty() && stopFlag.load()) break;

        LogEntry entry = std::move(logQueue.front());
        logQueue.pop();
        lock.unlock();

        logger.logMessage(entry.level, entry.message);
    }
}

int main(int argc, char* argv[]){
    std::string filename, level;
    if (argc == 3){
        filename = argv[1];
        level = argv[2];
    } else {
        std::cerr << "Incorrect arguments. Usage: " << argv[0] 
        << " <filename> <level> (DEBUG/INFO/ERROR)" << std::endl;
        return 1;
    }

    Level defaultLevel;
    if(!stringToLevel(level, defaultLevel)){
        std::cerr << "Unknown level: " << level << std::endl;
        return 1;
    }
    Logger logger(defaultLevel, filename);

    if (!logger.isOpen()) {
        std::cerr << "Fatal: failed to open log file: " << filename << std::endl;
        return 1;
    }

    std::thread worker(workerThread, std::ref(logger)); // 2 поток для записи в файл

    while (true) {
        std::cout << "> ";
        std::string line;
        
        if (!std::getline(std::cin, line)) { // EOF или ошибка — выходим
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                stopFlag = true;
            }
            cv.notify_all();
            break;
        }

        if (line == "exit") {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                stopFlag = true;
            }
            cv.notify_all();
            break;
        }

        if (line.empty()) {
            continue;
        }

        std::string message = line;
        Level msgLevel = defaultLevel;

        size_t lastSpace = line.find_last_of(' '); //последнее слово - ищем уровень важности

        if (lastSpace != std::string::npos) { // нашли уровень - делим его и текст сообщения
            std::string possibleLevel = line.substr(lastSpace + 1);
            Level parsed;
            if (stringToLevel(possibleLevel, parsed)) {
                msgLevel = parsed;
                message = line.substr(0, lastSpace);
            }
        }
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            logQueue.push({message, msgLevel});
        }
        cv.notify_one();
    }
    cv.notify_all();
    worker.join();
    return 0;
}