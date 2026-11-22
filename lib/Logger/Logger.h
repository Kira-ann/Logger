#pragma once
#include <mutex>
#include <fstream>
#include <thread>
#include <string>
#include <optional>
#include "LogLevel.h"
#include "BlockingQueue.h"

class Logger {
    BlockingQueue mes_queue_;
    LogLevel curr_log_level_;
    std::ofstream file_;
    std::mutex mutex_logger_;
    bool valid_ = true;

    //для потоков
    std::stop_source source_; // создание сигнализатора, что потоку - обработчику пока прекращать работу
    std::stop_token token_ = source_.get_token();
    std::jthread writer;

    public:
    bool is_valid() const;

    Logger(LogLevel input_level, std::string filename, bool input_append);

    ~Logger();

    void set_default_level(LogLevel input_level);

    void log(const std::string& text, std::optional<LogLevel> level);
};