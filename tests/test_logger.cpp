#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "Logger.h"
#include "LogLevel.h"

namespace {

class TempLogFile {
public:
    TempLogFile() : path_(MakeUniquePath()) {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }
    ~TempLogFile() {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }

    TempLogFile(const TempLogFile&) = delete;
    TempLogFile& operator=(const TempLogFile&) = delete;

    const std::filesystem::path& path() const { return path_; }
    std::string path_string() const { return path_.string(); }

private:
    static std::filesystem::path MakeUniquePath() {
        static std::atomic<unsigned long long> counter{0};
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        auto name = "logger_test_" + std::to_string(now) + "_" +
                    std::to_string(counter.fetch_add(1)) + ".log";
        return std::filesystem::temp_directory_path() / name;
    }

    std::filesystem::path path_;
};

std::vector<std::string> ReadLines(const std::filesystem::path& path) {
    std::ifstream in(path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    return lines;
}

} // namespace

class LoggerTest : public ::testing::Test {
protected:
    TempLogFile tmp_;
};

TEST_F(LoggerTest, WritesMessageWithTimestampLevelAndText) {
    {
        Logger logger(LogLevel::DEBUG, tmp_.path_string(), false);
        ASSERT_TRUE(logger.is_valid());
        logger.log("hello world", LogLevel::INFO);
    }

    auto lines = ReadLines(tmp_.path());
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_TRUE(std::regex_match(
        lines[0],
        std::regex(R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\] \[INFO\] hello world)")));
}

TEST_F(LoggerTest, FiltersMessagesBelowDefaultLevel) {
    {
        Logger logger(LogLevel::WARN, tmp_.path_string(), false);
        logger.log("debug message", LogLevel::DEBUG);
        logger.log("info message",  LogLevel::INFO);
        logger.log("warn message",  LogLevel::WARN);
        logger.log("error message", LogLevel::ERROR);
    }

    auto lines = ReadLines(tmp_.path());
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_NE(lines[0].find("[WARN] warn message"),   std::string::npos);
    EXPECT_NE(lines[1].find("[ERROR] error message"), std::string::npos);
}

TEST_F(LoggerTest, UsesDefaultLevelWhenLevelIsOmitted) {
    {
        Logger logger(LogLevel::WARN, tmp_.path_string(), false);
        logger.log("something", std::nullopt);
    }

    auto lines = ReadLines(tmp_.path());
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_NE(lines[0].find("[WARN] something"), std::string::npos);
}

TEST_F(LoggerTest, SetDefaultLevelLowersThreshold) {
    {
        Logger logger(LogLevel::ERROR, tmp_.path_string(), false);
        logger.log("before", LogLevel::INFO);   // отсечётся
        logger.set_default_level(LogLevel::INFO);
        logger.log("after", LogLevel::INFO);    // запишется
    }

    auto lines = ReadLines(tmp_.path());
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_NE(lines[0].find("[INFO] after"), std::string::npos);
}

TEST_F(LoggerTest, SetDefaultLevelRaisesThreshold) {
    {
        Logger logger(LogLevel::DEBUG, tmp_.path_string(), false);
        logger.log("info before", LogLevel::INFO);  // запишется
        logger.set_default_level(LogLevel::ERROR);
        logger.log("info after", LogLevel::INFO);   // отсечётся
        logger.log("error after", LogLevel::ERROR); // запишется
    }

    auto lines = ReadLines(tmp_.path());
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_NE(lines[0].find("[INFO] info before"),   std::string::npos);
    EXPECT_NE(lines[1].find("[ERROR] error after"),  std::string::npos);
}

TEST_F(LoggerTest, TruncateModeClearsExistingContent) {
    {
        std::ofstream out(tmp_.path());
        out << "old line\n";
    }
    {
        Logger logger(LogLevel::DEBUG, tmp_.path_string(), false);
        logger.log("new line", LogLevel::INFO);
    }

    auto lines = ReadLines(tmp_.path());
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_NE(lines[0].find("new line"), std::string::npos);
}

TEST_F(LoggerTest, AppendModePreservesExistingContent) {
    {
        std::ofstream out(tmp_.path());
        out << "old line\n";
    }
    {
        Logger logger(LogLevel::DEBUG, tmp_.path_string(), true);
        logger.log("new line", LogLevel::INFO);
    }

    auto lines = ReadLines(tmp_.path());
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0], "old line");
    EXPECT_NE(lines[1].find("new line"), std::string::npos);
}

TEST_F(LoggerTest, InvalidPathMakesLoggerInvalid) {
    auto bad_dir = tmp_.path().parent_path() / "no_such_dir_logger_tests";
    std::error_code ec;
    std::filesystem::remove_all(bad_dir, ec);

    Logger logger(LogLevel::INFO, (bad_dir / "log.txt").string(), false);
    EXPECT_FALSE(logger.is_valid());
}

TEST_F(LoggerTest, PreservesOrderFromSingleThread) {
    constexpr int kCount = 100;
    {
        Logger logger(LogLevel::DEBUG, tmp_.path_string(), false);
        for (int i = 0; i < kCount; ++i) {
            logger.log("msg_" + std::to_string(i), LogLevel::INFO);
        }
    }

    auto lines = ReadLines(tmp_.path());
    ASSERT_EQ(lines.size(), static_cast<size_t>(kCount));
    for (int i = 0; i < kCount; ++i) {
        EXPECT_TRUE(lines[i].ends_with("msg_" + std::to_string(i))) << lines[i];
    }
}

TEST_F(LoggerTest, DestructorDoesNotLoseQueuedMessages) {
    constexpr int kCount = 1000;
    {
        Logger logger(LogLevel::DEBUG, tmp_.path_string(), false);
        for (int i = 0; i < kCount; ++i) {
            logger.log("msg_" + std::to_string(i), LogLevel::INFO);
        }
    } // сразу уничтожение, очередь не пустая

    EXPECT_EQ(ReadLines(tmp_.path()).size(), static_cast<size_t>(kCount));
}

TEST_F(LoggerTest, ConcurrentWritersDoNotLoseOrDuplicateMessages) {
    constexpr int kThreads = 4;
    constexpr int kPerThread = 250;

    std::set<std::string> expected;
    {
        Logger logger(LogLevel::DEBUG, tmp_.path_string(), false);
        std::vector<std::thread> workers;
        for (int t = 0; t < kThreads; ++t) {
            for (int i = 0; i < kPerThread; ++i) {
                expected.insert("t" + std::to_string(t) + "_" + std::to_string(i));
            }
            workers.emplace_back([&logger, t] {
                for (int i = 0; i < kPerThread; ++i) {
                    logger.log("t" + std::to_string(t) + "_" + std::to_string(i),
                               LogLevel::INFO);
                }
            });
        }
        for (auto& th : workers) th.join();
    }

    auto lines = ReadLines(tmp_.path());
    ASSERT_EQ(lines.size(), expected.size());

    std::set<std::string> seen;
    for (const auto& line : lines) {
        auto pos = line.rfind("] ");
        ASSERT_NE(pos, std::string::npos) << line;
        seen.insert(line.substr(pos + 2));
    }
    EXPECT_EQ(seen, expected);
}

TEST_F(LoggerTest, MessageIsVisibleBeforeLoggerDestruction) {
    Logger logger(LogLevel::DEBUG, tmp_.path_string(), false);
    ASSERT_TRUE(logger.is_valid());

    logger.log("already here", LogLevel::INFO);

    std::vector<std::string> lines;
    for (int i = 0; i < 200; ++i) {
        lines = ReadLines(tmp_.path());
        if (!lines.empty()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::error_code ec;
    std::cerr << "DEBUG: file size = "
              << std::filesystem::file_size(tmp_.path(), ec)
              << ", error = " << ec.message() << '\n';

    ASSERT_EQ(lines.size(), 1u);
    EXPECT_NE(lines[0].find("already here"), std::string::npos);
}