#include "Logger.h"

bool Logger::is_valid() const {return valid_;}

Logger::Logger(LogLevel input_level, std::string filename, bool input_append){
    if (input_append){
        file_.open(filename, std::ios::app);
    } else {file_.open(filename);}
    if (!file_.is_open()){valid_ = false; return;}
    curr_log_level_ = input_level;
    writer = std::jthread([this](){
        while(true){
            auto result = mes_queue_.pop(token_);
            if (result){
                auto time = std::format("{:%Y-%m-%d %H:%M:%S}", 
                std::chrono::floor<std::chrono::seconds>(result.value().time));
                auto level = result.value().level;
                auto text = result.value().text;
                file_ << "[" << time << "] [" << toString(level)  << "] " << text << '\n';
                file_.flush();
            }
            else {break;}
        } 
    });
}

Logger::~Logger(){
    source_.request_stop(); 
    // ?? очередь пустая, файл закрывать не надо - хз что чистить - пустой пусть будет
}

void Logger::set_default_level(LogLevel input_level){
    std::lock_guard<std::mutex> lock(mutex_logger_);
    curr_log_level_ = input_level;
}

void Logger::log(const std::string& text, std::optional<LogLevel> level){
    LogLevel curr_level;
    {   
        std::lock_guard<std::mutex> lock(mutex_logger_);
        curr_level = curr_log_level_;
    }
    LogLevel msg_level;
    if (level){msg_level = level.value();}
    else {msg_level = curr_level;}
    if (msg_level >= curr_level){
        mes_queue_.push({
            .time = std::chrono::system_clock::now(),
            .text = text,
            .level = msg_level
        });
    }
}
