#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <set>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

#include "BlockingQueue.h"

namespace {

LogMessage MakeMessage(std::string text, LogLevel level = LogLevel::INFO) {
    return LogMessage{
        .time  = std::chrono::system_clock::now(),
        .text  = std::move(text),
        .level = level,
    };
}

} // namespace

TEST(BlockingQueueTest, PushThenPopReturnsSameMessage) {
    BlockingQueue queue;
    queue.push(MakeMessage("hello", LogLevel::WARN));

    std::stop_source source;
    auto msg = queue.pop(source.get_token());

    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->text, "hello");
    EXPECT_EQ(msg->level, LogLevel::WARN);
}

TEST(BlockingQueueTest, PopPreservesFifoOrder) {
    BlockingQueue queue;
    queue.push(MakeMessage("first"));
    queue.push(MakeMessage("second"));
    queue.push(MakeMessage("third"));

    std::stop_source source;
    EXPECT_EQ(queue.pop(source.get_token())->text, "first");
    EXPECT_EQ(queue.pop(source.get_token())->text, "second");
    EXPECT_EQ(queue.pop(source.get_token())->text, "third");
}

TEST(BlockingQueueTest, PopReturnsNulloptOnStopWhenQueueIsEmpty) {
    BlockingQueue queue;
    std::stop_source source;
    source.request_stop();

    EXPECT_FALSE(queue.pop(source.get_token()).has_value());
}

// Фиксирует текущее поведение: остановка не выбрасывает уже лежащие сообщения.
TEST(BlockingQueueTest, PopDrainsQueuedMessagesAfterStop) {
    BlockingQueue queue;
    queue.push(MakeMessage("a"));
    queue.push(MakeMessage("b"));

    std::stop_source source;
    source.request_stop();

    EXPECT_EQ(queue.pop(source.get_token())->text, "a");
    EXPECT_EQ(queue.pop(source.get_token())->text, "b");
    EXPECT_FALSE(queue.pop(source.get_token()).has_value());
}

TEST(BlockingQueueTest, PopBlocksUntilPush) {
    using namespace std::chrono_literals;

    BlockingQueue queue;
    std::stop_source source;

    auto future = std::async(std::launch::async, [&] {
        return queue.pop(source.get_token());
    });

    EXPECT_EQ(future.wait_for(50ms), std::future_status::timeout);

    queue.push(MakeMessage("late"));

    auto msg = future.get();
    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->text, "late");
}

TEST(BlockingQueueTest, ConcurrentProducersDoNotLoseMessages) {
    BlockingQueue queue;
    constexpr int kProducers = 4;
    constexpr int kPerProducer = 250;
    constexpr int kTotal = kProducers * kPerProducer;

    std::vector<std::thread> producers;
    for (int t = 0; t < kProducers; ++t) {
        producers.emplace_back([&queue, t] {
            for (int i = 0; i < kPerProducer; ++i) {
                queue.push(MakeMessage("msg_" + std::to_string(t) + "_" + std::to_string(i)));
            }
        });
    }
    for (auto& th : producers) th.join();

    std::stop_source source;
    std::set<std::string> seen;
    for (int i = 0; i < kTotal; ++i) {
        auto msg = queue.pop(source.get_token());
        ASSERT_TRUE(msg.has_value());
        seen.insert(msg->text);
    }
    EXPECT_EQ(seen.size(), static_cast<size_t>(kTotal));
}