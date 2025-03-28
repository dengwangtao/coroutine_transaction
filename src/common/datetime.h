#pragma once
#ifndef _COMMON_DATETIME_H_
#define _COMMON_DATETIME_H_

#include <chrono>
#include <sys/time.h>
#include <time.h>
#include <string>
#include "common_def.h"


// 1970/1/1 是周四，因此这里WeekOffset为+3天
constexpr int64_t kUnixWeekdayOffset = 3 * SECOND_PER_DAY;

class DateTime
{
public:
    static const std::string kDateTimeFormat;
    static const std::string kDateTimeTzFormat;
    static const std::string kOffsetFormat;

    /**
     * @brief 使用当前时间构造一个DateTime对象
     */
    DateTime();

    explicit DateTime(int64_t timestamp_sec);

    explicit DateTime(const struct timeval &tv);

    explicit DateTime(const struct tm& tm);

    DateTime(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t minute, int32_t second);




public: // static functions

    static int64_t GetConfigTZOffsetSec()
    {
        return config_tz_sec_;
    }
    static void SetConfigTZOffsetSec(int32_t seconds)
    {
        config_tz_sec_ = seconds;
    }


    template <typename Duration>
    static int64_t GetNowSteadyTime() {
        return std::chrono::duration_cast<Duration>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    static int64_t GetNowSteadySec()  { return GetNowSteadyTime<std::chrono::seconds>(); }
    static int64_t GetNowSteadyMSec() { return GetNowSteadyTime<std::chrono::milliseconds>(); }
    static int64_t GetNowSteadyUSec() { return GetNowSteadyTime<std::chrono::microseconds>(); }
    static int64_t GetNowSteadyNSec() { return GetNowSteadyTime<std::chrono::nanoseconds>(); }


    // 系统时间（日历时间）
    template <typename Duration>
    static int64_t GetSystemTime() {
        return std::chrono::duration_cast<Duration>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    static int64_t GetSystemSec()  { return GetSystemTime<std::chrono::seconds>(); }
    static int64_t GetSystemMSec() { return GetSystemTime<std::chrono::milliseconds>(); }
    static int64_t GetSystemUSec() { return GetSystemTime<std::chrono::microseconds>(); }
    static int64_t GetSystemNSec() { return GetSystemTime<std::chrono::nanoseconds>(); }

    static std::string Format(int64_t timestamp_sec, const std::string& format = kDateTimeFormat);
    static std::string NowS() { return Format(GetSystemSec()); }



    static int64_t GetHourStartTime(int64_t time_in_sec, int64_t offset = 0, int64_t tz_offset = GetConfigTZOffsetSec())
    {
        return (time_in_sec + tz_offset - offset) / SECOND_PER_HOUR * SECOND_PER_HOUR - (tz_offset - offset);
    }

    static int64_t GetDayStartTime(int64_t time_in_sec, int64_t offset = 0, int64_t tz_offset = GetConfigTZOffsetSec())
    {
        return (time_in_sec + tz_offset - offset) / SECOND_PER_DAY * SECOND_PER_DAY - (tz_offset - offset);
    }

    static int64_t GetWeekStartTime(int64_t time_in_sec, int64_t offset = 0, int64_t tz_offset = GetConfigTZOffsetSec())
    {
        return (time_in_sec + tz_offset - offset + kUnixWeekdayOffset) / SECOND_PER_WEEK * SECOND_PER_WEEK -
               (tz_offset - offset) - kUnixWeekdayOffset;
    }

    static uint32_t GetPassedDay(int64_t begin_time_sec, int64_t cur_time_sec, int64_t offset = 0)
    {
        if (begin_time_sec >= cur_time_sec)
        {
            return 0;
        }
        return (GetDayStartTime(cur_time_sec, offset) - GetDayStartTime(begin_time_sec, offset)) / SECOND_PER_DAY;
    }

public:

    // Use GetDayOffset to convert (str) offset_str(e.g. "08:00:00") to (int) offset
    static bool IsSameDay(int64_t now, int64_t old_time, int64_t offset = 0);
    static bool IsPassDay(int64_t now, int64_t old, int64_t offset = 0);
    static bool IsSameWeek(int64_t now, int64_t old, int64_t offset = 0);
    static bool IsPassWeek(int64_t now, int64_t old, int64_t offset = 0);
    static bool IsSameMonth(int64_t now, int64_t old_time, int64_t offset = 0);
    static bool IsPassMonth(int64_t now, int64_t old_time, int64_t offset = 0);

    static int32_t GetMonthDay(int64_t time, int64_t offset = 0);

private:

    static int64_t config_tz_sec_;
};

#endif // _COMMON_DATETIME_H_