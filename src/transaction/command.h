#pragma once

#include <typeinfo>
#include <string>
#include <vector>
#include "common_def.h"

const s32 kWaitMsgDefaultTimeout = (30 * 1000);

const s32 kMaxEventSizeInCommand = 6;

constexpr size_t kMaxDemangledClassNameSize = 32;

class TransactionInstance;

class Command
{
public:
    using EventIdVec = std::vector<s32>;

    explicit Command(const EventIdVec& event_id_vec, s32 timeout_ms = kWaitMsgDefaultTimeout)
        : do_event_id_vec_(event_id_vec.begin(), event_id_vec.end())
        , timeout_ms_(timeout_ms)
    {
        demangled_cls_name_[0] = '\0';
    }

    explicit Command(s32 event_id = 0, s32 timeout_ms = kWaitMsgDefaultTimeout)
        : Command(EventIdVec { event_id }, timeout_ms)
    {
    }

    virtual ~Command() = default;

    const char* GetName() const;

public:
    virtual s32 DoAndWait(TransactionInstance &inst);

    virtual s32 Wait(TransactionInstance &inst);

    // It doesn't matter if undo fails. Undo should never block.
    virtual void Undo(TransactionInstance& inst);

    virtual s32 OnRecvMsgEvent(TransactionInstance& inst, s32 event_id) = 0;

    virtual s32 OnTimeout(TransactionInstance& inst);

    inline const EventIdVec& do_event_id_vec() const
    {
        return do_event_id_vec_;
    }

protected:
    virtual s32 Do(TransactionInstance& inst) = 0;

    virtual s32 WaitForEvent(TransactionInstance& inst, const EventIdVec& msg_event_id_vec);

    const EventIdVec do_event_id_vec_;
    const s32 timeout_ms_;
    mutable char demangled_cls_name_[kMaxDemangledClassNameSize];
};


