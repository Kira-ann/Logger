#pragma once

#include <optional>
#include <chrono>
#include <string>
#include <queue>
#include <condition_variable>
#include <mutex>
#include "LogLevel.h"

struct LogMessage {
    std::chrono::system_clock::time_point time;
    std::string text;
    LogLevel level;
};

class BlockingQueue{
    std::queue<LogMessage> q;
    std::mutex mutex_queue_;
    std::condition_variable_any cv_;

    public:
    void push(const LogMessage& curr_msg);
    std::optional<LogMessage> pop(std::stop_token st);
};