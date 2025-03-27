#pragma once

#ifndef SRC_COMMON_COMMON_DEF_H_
#define SRC_COMMON_COMMON_DEF_H_

#include <cstdint>
#include <iostream>
#include <iomanip>
#include "error_define.pb.h"


using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s32 = int32_t;
using s64 = int64_t;



#define INVALID_TIMER_ID      0ULL


struct uuid_t
{
    u64 high_;
    u64 low_;
    bool operator==(const uuid_t &other) const
    {
        return high_ == other.high_ && low_ == other.low_;
    }
    bool operator<(const uuid_t &other) const
    {
        return high_ < other.high_ ? true : (high_ > other.high_ ? false : low_ < other.low_);
    }
    bool operator!=(const uuid_t &other) const
    {
        return high_ != other.high_ || low_ != other.low_;
    }
    bool empty() const
    {
        return high_ == 0 && low_ == 0;
    }

    const char *c_str() const;
};













// 日志流辅助类
class LogStream {
public:
    LogStream(const char* file, int line, const char* fname, const char* level);
    
    ~LogStream();
    
    // 重载 << 运算符
    template<typename T>
    LogStream& operator<<(const T& value) {
        std::cout << value;
        return *this;
    }
};

// 定义宏，将绝对路径转换为相对路径
#define __RELATIVE_FILE__ ( \
    strstr(__FILE__, PROJECT_ROOT_DIR) ? \
    strstr(__FILE__, PROJECT_ROOT_DIR) + strlen(PROJECT_ROOT_DIR) : \
    __FILE__ \
)

// 函数名宏
#ifdef USE_PRETTY_FUNCTION
#define MY_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
#define MY_PRETTY_FUNCTION __FUNCTION__
#endif

// 日志宏（支持自动换行）
#define LogFatal() LogStream(__RELATIVE_FILE__, __LINE__, MY_PRETTY_FUNCTION, "FATAL")
#define LogError() LogStream(__RELATIVE_FILE__, __LINE__, MY_PRETTY_FUNCTION, "ERROR")
#define LogWarn()  LogStream(__RELATIVE_FILE__, __LINE__, MY_PRETTY_FUNCTION, "WARN_")
#define LogInfo()  LogStream(__RELATIVE_FILE__, __LINE__, MY_PRETTY_FUNCTION, "INFO_")
#define LogDebug() LogStream(__RELATIVE_FILE__, __LINE__, MY_PRETTY_FUNCTION, "DEBUG")
#define LogTrace() LogStream(__RELATIVE_FILE__, __LINE__, MY_PRETTY_FUNCTION, "TRACE")



#define _LogK(k) #k << "<" << k << "> "
#define _LogKV(k, v) #k << "<" << v << "> "
#define _LogKV2(k, v1, v2) #k << "<" << v1 << "," << v2 << "> "







#define likely(x)	__builtin_expect(((x) != 0),1)
#define unlikely(x)	__builtin_expect(((x) != 0),0)








constexpr int32_t NS_PER_US = 1000;
constexpr int32_t US_PER_MS = 1000;
constexpr int32_t MS_PER_SECOND = 1000;
constexpr int64_t US_PER_SECOND = US_PER_MS * MS_PER_SECOND;
constexpr int64_t NS_PER_MS = NS_PER_US * US_PER_MS;
constexpr int64_t NS_PER_SECOND = NS_PER_US * US_PER_MS * MS_PER_SECOND;
constexpr int32_t SECOND_PER_MINUTE = 60;
constexpr int32_t MINUTE_PER_HOUR = 60;
constexpr int32_t SECOND_PER_HOUR = MINUTE_PER_HOUR * SECOND_PER_MINUTE;
constexpr int32_t HOUR_PER_DAY = 24;
constexpr int32_t SECOND_PER_DAY = HOUR_PER_DAY * SECOND_PER_HOUR;
constexpr int32_t DAY_PER_WEEK = 7;
constexpr int32_t SECOND_PER_WEEK = DAY_PER_WEEK * SECOND_PER_DAY;




class CommonUtil
{
    static std::string GetDemangledName(const char *name);
    static int GetDemangledName(const char *name, char *buf, size_t len);

public:
    template <typename T>
    static std::string GetDemangledName(const T *p)
    {
        return GetDemangledName(typeid(*p).name());
    }

    template <typename T>
    static int GetDemangledName(const T *p, char *buf, size_t len)
    {
        return GetDemangledName(typeid(*p).name(), buf, len);
    }


    template <typename T, s32 N>
    inline static constexpr s32 array_size(const T (&)[N])
    {
        return N;
    }

    template<typename T>
    inline static T Clamp(T v, T lo, T hi)
    {
        return v < lo ? lo : ((hi < v) ? hi : v);
    }
};

#endif  // SRC_COMMON_COMMON_DEF_H_