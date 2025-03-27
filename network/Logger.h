#pragma once

#include "noncopyable.h"
#include <string>

namespace dwt {

// 定义日志级别
enum class LogLevel {

    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS
};


class Logger: noncopyable {

public:
    // 获取单例对象
    static Logger& getInstance();

    // 设置日志级别
    void setLogLevel(LogLevel level);

    // 写日志
    void log(std::string msg);

private:

    LogLevel m_logLevel;

    Logger() {}
};


#ifdef DWT_DEBUG

    #define LOG_DEBUG(logMsgFormat, ...)                    \
    do {                                                    \
        dwt::Logger& logger = dwt::Logger::getInstance();   \
        logger.setLogLevel(dwt::LogLevel::DEBUG);           \
        char buf[1024] = {0};                               \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
    } while(0);

#else
    #define LOG_DEBUG(logMsgFormat, ...) 
#endif




#define LOG_INFO(logMsgFormat, ...)                     \
do {                                                    \
    dwt::Logger& logger = dwt::Logger::getInstance();   \
    logger.setLogLevel(dwt::LogLevel::INFO);            \
    char buf[1024] = {0};                               \
    snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__);   \
    logger.log(buf);                                    \
} while(0);

#define LOG_WARN(logMsgFormat, ...)                     \
do {                                                    \
    dwt::Logger& logger = dwt::Logger::getInstance();   \
    logger.setLogLevel(dwt::LogLevel::WARN);            \
    char buf[1024] = {0};                               \
    snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__);   \
    logger.log(buf);                                    \
} while(0);

#define LOG_ERROR(logMsgFormat, ...)                    \
do {                                                    \
    dwt::Logger& logger = dwt::Logger::getInstance();   \
    logger.setLogLevel(dwt::LogLevel::ERROR);           \
    char buf[1024] = {0};                               \
    snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__);   \
    logger.log(buf);                                    \
} while(0);

#define LOG_FATAL(logMsgFormat, ...)                    \
do {                                                    \
    dwt::Logger& logger = dwt::Logger::getInstance();   \
    logger.setLogLevel(dwt::LogLevel::FATAL);           \
    char buf[1024] = {0};                               \
    snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__);   \
    logger.log(buf);                                    \
    exit(-1);                                           \
} while(0);



}