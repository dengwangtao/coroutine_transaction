#include "datetime.h"


const std::string DateTime::kDateTimeFormat = "%Y-%m-%d %H:%M:%S";
const std::string DateTime::kDateTimeTzFormat = "%Y-%m-%d %H:%M:%S %z";
const std::string DateTime::kOffsetFormat = "%H:%M:%S";


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

