#pragma once

#ifndef SRC_COMMON_COMMON_DEF_H_
#define SRC_COMMON_COMMON_DEF_H_

#include <cstdint>
#include <iostream>
#include "error_define.pb.h"

using u32 = uint32_t;
using u64 = uint64_t;
using s32 = int32_t;
using s64 = int64_t;



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
    LogStream(const char* file, int line, const char* level) {
        std::cout << "[" << level << "] (" << file << ":" << line << ") ";
    }
    
    ~LogStream() {
        std::cout << std::endl;  // 自动追加换行
    }
    
    // 重载 << 运算符
    template<typename T>
    LogStream& operator<<(const T& value) {
        std::cout << value;
        return *this;
    }
};

// 日志宏（支持自动换行）
#define LogFatal() LogStream(__FILE__, __LINE__, "Fatal")
#define LogError() LogStream(__FILE__, __LINE__, "Error")
#define LogWarn()  LogStream(__FILE__, __LINE__, "Warn ")
#define LogInfo()  LogStream(__FILE__, __LINE__, "Info ")
#define LogDebug() LogStream(__FILE__, __LINE__, "Debug")
#define LogTrace() LogStream(__FILE__, __LINE__, "Trace")












#define likely(x)	__builtin_expect(((x) != 0),1)
#define unlikely(x)	__builtin_expect(((x) != 0),0)









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

};

#endif  // SRC_COMMON_COMMON_DEF_H_