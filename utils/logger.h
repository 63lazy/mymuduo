#pragma once
#include "noncopy.h"
#include <string>
#include <cstdio>

#define LOG_INFO() \
    do { \
        Logger &logger = Logger::instance(); \
        logger.set_level(INFO); \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), "%s:%d:%s", __FILE__, __LINE__, __FUNCTION__); \
        logger.log(buffer); \
    } while (0)

#define LOG_DEBUG() \
    do { \
        Logger &logger = Logger::instance(); \
        logger.set_level(DEBUG); \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), "%s:%d:%s", __FILE__, __LINE__, __FUNCTION__); \
        logger.log(buffer); \
    } while (0)

#define LOG_ERROR() \
    do { \
        Logger &logger = Logger::instance(); \
        logger.set_level(ERROR); \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), "%s:%d:%s", __FILE__, __LINE__, __FUNCTION__); \
        logger.log(buffer); \
    } while (0)

#define LOG_FATAL() \
    do { \
        Logger &logger = Logger::instance(); \
        logger.set_level(FATAL); \
        char buffer[1024]; \
        std::snprintf(buffer, sizeof(buffer), "%s:%d:%s", __FILE__, __LINE__, __FUNCTION__); \
        logger.log(buffer); \
        exit(-1); \
    } while (0)

// 日志类型
enum LogLevel {
    DEBUG,
    INFO,
    FATAL,
    ERROR
};

// 输出一个日志类
class Logger : NonCopyable {
public:
    static Logger& instance();
    void set_level(int level);
    void log(std::string msg);
private:
    int log_level;
    Logger() {};
};