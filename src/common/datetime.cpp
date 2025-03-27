#include "datetime.h"


const std::string DateTime::kDateTimeFormat = "%Y-%m-%d %H:%M:%S";
const std::string DateTime::kDateTimeTzFormat = "%Y-%m-%d %H:%M:%S %z";
const std::string DateTime::kOffsetFormat = "%H:%M:%S";


int64_t DateTime::config_tz_sec_ = 0;

bool DateTime::IsSameDay(int64_t now, int64_t old, int64_t offset)
{
    return GetDayStartTime(now, offset) == GetDayStartTime(old, offset);
}

bool DateTime::IsPassDay(int64_t now, int64_t old, int64_t offset)
{
    return GetDayStartTime(now, offset) > GetDayStartTime(old, offset);
}

bool DateTime::IsSameWeek(int64_t now, int64_t old, int64_t offset)
{
    return GetWeekStartTime(now, offset) == GetWeekStartTime(old, offset);
}

bool DateTime::IsPassWeek(int64_t now, int64_t old, int64_t offset)
{
    return GetWeekStartTime(now, offset) > GetWeekStartTime(old, offset);
}

std::string DateTime::Format(int64_t timestamp_sec, const std::string& format)
{
    time_t timestamp = timestamp_sec;
    struct tm tm_time;
    localtime_r(&timestamp, &tm_time);
    char buf[128];
    strftime(buf, sizeof(buf), format.c_str(), &tm_time);
    return std::string(buf);
}
