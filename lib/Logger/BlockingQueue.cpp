#include "BlockingQueue.h"
#include <utility>

void BlockingQueue::push(const LogMessage& curr_msg){
    {
        std::lock_guard<std::mutex> lock(mutex_queue_);
        q.push(curr_msg);
    }
    cv_.notify_one();
}

std::optional<LogMessage> BlockingQueue::pop(std::stop_token st){
    std::unique_lock<std::mutex> lock(mutex_queue_);
    bool has_data = cv_.wait(lock, st, [this] {return !q.empty(); }); //this - длязахвата поля класса
    // сначала проверяется предикат, после требование stop_token 
    if (!has_data){return std::nullopt;}
    LogMessage curr = std::move(q.front());
    q.pop();
    return curr;
}
