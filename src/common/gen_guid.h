#pragma once

#ifndef SRC_COMMON_GEN_GUID_H_
#define SRC_COMMON_GEN_GUID_H_

#include "common_def.h"


enum OBJ_TYPE
{
    OBJ_TYPE_INVALID = 0,
    OBJ_TYPE_TRANSACTION_INSTANCE = 1,
    OBJ_TYPE_COROUTINE = 2,
    OBJ_TYPE_TIMER = 3,
};

/**
 * 8 bit for type, 32 bits for time_t, 24 bit for seq
 */

constexpr int kTypeBits = 8;
constexpr int kTimestampBits = 32;
constexpr int kSeqBits = 24;

constexpr u64 kTypeMask = (1ULL << kTypeBits) - 1;
constexpr u64 kTimestampMask = (1ULL << kTimestampBits) - 1;
constexpr u64 kSeqMask = (1ULL << kSeqBits) - 1;

// 从 1970-01-01 00:00:00 到 2025-01-01 00:00:00 的秒数
constexpr u32 kSecFrom19700101To20250101 = 1735689600;




u64 GenGUID(OBJ_TYPE type);

std::string GUIDStr(u64 guid);

#endif