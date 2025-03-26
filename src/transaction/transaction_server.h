#pragma once
#ifndef SRC_TRANSACTION_TRANSACTION_SERVER_H_
#define SRC_TRANSACTION_TRANSACTION_SERVER_H_

#include "common_def.h"
#include "coroutine/coroutine_scheduler.h"
#include "transaction_mgr.h"
#include "svr_timer.h"

class Transaction;

class TransactionServer
{
public:
    TransactionServer();
    virtual ~TransactionServer();

    // 只有用到了事务且需要resume的server会需要, 因此提供了默认实现
    virtual Transaction* GetTranByType(s32 type) const
    {
        return nullptr;
    }

    CoroutineScheduler* co_scheduler() const
    {
        return scheduler_ptr_;
    }

    TranInstMgr* tran_mgr() const
    {
        return tran_mgr_ptr_;
    }

    TimerMgr* timer_mgr() const
    {
        return timer_mgr_ptr_;
    }
private:

    CoroutineScheduler* scheduler_ptr_ = nullptr;
    TranInstMgr* tran_mgr_ptr_ = nullptr;
    TimerMgr* timer_mgr_ptr_ = nullptr;

};


// TransactionServer* g_trans_server = nullptr;
std::unique_ptr<TransactionServer> g_trans_server_ptr;

#endif // SRC_TRANSACTION_TRANSACTION_SERVER_H_