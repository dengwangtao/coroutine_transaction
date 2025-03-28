#pragma once
#ifndef SRC_TRANSACTION_TRANSACTION_SERVER_H_
#define SRC_TRANSACTION_TRANSACTION_SERVER_H_

#include "common_def.h"
#include "coroutine/coroutine_scheduler.h"
#include "transaction_mgr.h"
#include "svr_timer.h"

class Transaction;
class TransactionInstance;

class TransactionServer
{
public:
    TransactionServer();
    virtual ~TransactionServer();


    virtual s32 Init();


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

    virtual s32 OnTick();

    // 注册定时器函数
    virtual s32 RegisterTimerFunc();

    s32 StartCommonTransaction(Transaction& transaction, u64 owner_id,
                               TransactionInstance** saved_inst, u64 param1 = 0,
                               u64 param2 = 0) const;

    s32 StartTransaction(TransactionInstance *inst) const;
    
    s32 SendMsgEventToTran(u64 tran_id, s32 event_id, void* data) const;
private:

    CoroutineScheduler* scheduler_ptr_ = nullptr;
    TranInstMgr* tran_mgr_ptr_ = nullptr;
    TimerMgr* timer_mgr_ptr_ = nullptr;

};


extern TransactionServer* g_trans_server_ptr;
// extern std::unique_ptr<TransactionServer> g_trans_server_ptr;

#endif // SRC_TRANSACTION_TRANSACTION_SERVER_H_