#include "gen_guid.h"
#include <chrono>


// 从 1970-01-01 00:00:00 到 2025-01-01 00:00:00 的秒数
constexpr u32 kSecFrom19700101To20250101 = 1735689600;

u64 GenGUID()
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

    constexpr int kPart01Bits = 4;
    constexpr int kPart02Bits = 12;
    constexpr int kTimestampBits = 30;
    constexpr int kSeqBits = 18;

    constexpr u64 kPart01Mask = (1ULL << kPart01Bits) - 1;
    constexpr u64 kPart02Mask = (1ULL << kPart02Bits) - 1;
    constexpr u64 kTimestampMask = (1ULL << kTimestampBits) - 1;
    constexpr u64 kSeqMask = (1ULL << kSeqBits) - 1;

    // 缓存结果
    u64 part1 = static_cast<u64>(0) & kPart01Mask;
    u64 part2 = static_cast<u64>(0) & kPart02Mask;
    u64 timestamp = (static_cast<u64>(cur_s - kSecFrom19700101To20250101)) & kTimestampMask;
    u64 seq = static_cast<u64>(cur_seq) & kSeqMask;

    // 构建 GUID
    u64 guid = 0;
    guid |= part1 << (64 - kPart01Bits);
    guid |= part2 << (64 - kPart01Bits - kPart02Bits);
    guid |= timestamp << kSeqBits;
    guid |= seq;

    return guid;
}
