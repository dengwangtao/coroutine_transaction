#pragma once
#ifndef SRC_TRANSACTION_TRANSACTION_INSTANCE_H_
#define SRC_TRANSACTION_TRANSACTION_INSTANCE_H_


#include "transaction_comm.h"


class Transaction;

// 具体的事务实例
class TransactionInstance
{
    friend Transaction;
    static constexpr int kMaxEventCount = 10;

public:
    TransactionInstance(s32 type, u64 owner_id);
    virtual ~TransactionInstance();

public:
    void SafeRelease();

    s32 SendMsgEvent(s32 type, void* msg);

    s32 Wait(s32* events, s32 event_count, s32 timeout_ms);

    s32 Abort();
    s32 ProcDefaultEvents(bool& is_proc);

    s32 Resume();

    void SetParams(u64 param1, u64 param2)
    {
        param1_ = param1;
        param2_ = param2;
    }

    bool IsDefaultEvent(s32 event_id) const
    {
        return (event_id == E_TRANSACTION_EVENT_TYPE_TIMEOUT ||
                event_id == E_TRANSACTION_EVENT_TYPE_ABORT);
    }

    void SetEventArg(s32 type, void* msg);

    TransactionEventArg& event_arg() { return event_arg_; }

    u64 owner_id() const { return owner_id_; }
    u64 id() const { return id_; }
    s32 type() const { return type_; }
    s32 event_type() const { return event_type_; }
    u64 coroutine_id() const { return coroutine_id_; }
    void set_coroutine_id(u64 id) { coroutine_id_ = id; }

    // 完成不代表所有command都跑完了, 可能提前完成. 如果需要区分, 增加接口同时
    // 检查curr_index_ >= cmd_array_.size()
    bool is_complete() const { return is_complete_; }
    void complete() { is_complete_ = true; }

    bool should_wait_current_cmd() const { return should_wait_current_cmd_; }
    void set_should_wait_current_cmd(bool v) { should_wait_current_cmd_ = v; }

    s32 stack_size() const { return stack_size_; }
    void set_stack_size(s32 size) { stack_size_ = size; }

    bool is_delay_destroying() const { return is_delay_destroying_; }
    void set_is_delay_destroying(bool v) { is_delay_destroying_ = v; }

    u64 param1() const { return param1_; }
    u64 param2() const { return param2_; }

    s32 fail_reason() const { return fail_reason_; }
    s32 fail_index() const { return fail_index_; }
    bool IsFailed() const { return fail_reason_ != 0; }

    bool IsRunning() const
    {
        return curr_index_ >= 0 && (!is_complete() && !IsFailed());
    }

    static void TransactionOnTimeout(u64 timer_id, void *data, size_t data_len);

private:
    void set_fail_reason(s32 reason) { fail_reason_ = reason; }

    bool is_in_undo() const { return is_in_undo_; }
    void set_is_in_undo(bool is_in) { is_in_undo_ = is_in; }

    s32 curr_index() const { return curr_index_; }
    void set_curr_index(s32 v) { curr_index_ = v; }

    void set_fail_index(s32 v) { fail_index_ = v; }

    s32 waiting_index() const { return waiting_index_; }
    void set_waiting_index(s32 v) { waiting_index_ = v; }

    // 释放此对象的内存. 不要直接call, call SafeRelease
    virtual void Release();

    const u64 id_ = 0;                // 自身id
    const u64 owner_id_ = 0;    // 拥有这个事务的对象ID
    u64 timer_id_ = 0; // 定时器ID, 用来处理超时
    u64 coroutine_id_ = 0; // 对应协程ID, 每个事务只对应一个协程
    u64 param1_ = 0;       // 可以携带额外两个参数
    u64 param2_ = 0;
    TransactionEventArg event_arg_; // 事件携带的参数
    const s32 type_;
    s32 curr_index_ = -1;
    s32 waiting_index_ = -1;
    s32 fail_index_ = -1; // -1或者curr_idx
    s32 event_count_ = 0;
    s32 events_[kMaxEventCount];
    s32 event_type_;  // 当前激活的事件类型
    s32 fail_reason_ = 0; // 失败原因，对应错误码
    s32 stack_size_ = 0;   // 堆栈大小
    bool is_in_undo_ = false; // 是否在执行undo中
    bool is_complete_ = false;  // 事务是否完成了
    bool is_delay_destroying_ = false;

    // 有些命令在某些情况下可能是个空操作, 不需要等待
    bool should_wait_current_cmd_ = true;
};


#endif //TRANSACTION_TRANSACTION_INSTANCE_H_