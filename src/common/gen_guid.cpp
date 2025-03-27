#include "gen_guid.h"
#include <chrono>
#include "datetime.h"
#include <sstream>

static const char* OBJ_TYPE_Name(OBJ_TYPE type)
{
    switch (type)
    {
        case OBJ_TYPE_INVALID:
            return "INVALID";
        case OBJ_TYPE_TRANSACTION_INSTANCE:
            return "TRANSACTION_INSTANCE";
        case OBJ_TYPE_COROUTINE:
            return "COROUTINE";
        case OBJ_TYPE_TIMER:
            return "TIMER";
        default:
            return "UNKNOWN";
    }
}

std::string GUIDStr(u64 guid)
{
    u64 type = (guid >> (64 - kTypeBits)) & kTypeMask;
    u64 timestamp = (guid >> kSeqBits) & kTimestampMask;
    timestamp = kSecFrom19700101To20250101 + timestamp;
    u64 seq = guid & kSeqMask;
    
    DateTime::Format(timestamp);

    std::ostringstream oss;
    oss << "{"
        << "type="<< OBJ_TYPE_Name(static_cast<OBJ_TYPE>(type))
        << ", timestamp="<< DateTime::Format(timestamp)
        << ", seq="<< seq
        << "}";
    
    return oss.str();
}

u64 GenGUID(OBJ_TYPE type)
{
    static u32 last_s = 0, cur_seq = 0;

    u32 cur_s =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (last_s != cur_s)
    {
        cur_seq = 0;
        last_s = cur_s;
    }
    else
    {
        ++ cur_seq;
    }

    // 缓存结果
    u64 utype = static_cast<u64>(type) & kTypeMask;
    u64 timestamp = (static_cast<u64>(cur_s - kSecFrom19700101To20250101)) & kTimestampMask;
    u64 seq = static_cast<u64>(cur_seq) & kSeqMask;

    // 构建 GUID
    u64 guid = 0;
    guid |= utype << (64 - kTypeBits);
    guid |= timestamp << kSeqBits;
    guid |= seq;

    return guid;
}
