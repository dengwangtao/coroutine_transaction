#include "Logger.h"

#include <iostream>
#include "Timestamp.h"

namespace dwt {

// 获取单例对象
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// 设置日志级别
void Logger::setLogLevel(LogLevel level) {
    m_logLevel = level;
}

const char* LogLevelName[(int)LogLevel::NUM_LOG_LEVELS] =
{
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

// 写日志
void Logger::log(std::string msg) {

    /**
     * [级别信息] 20240528 09:46:48 : msg
    */

    // 日志等级
    std::cout << "[" << LogLevelName[(int)m_logLevel] << "] ";

    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;

}


}