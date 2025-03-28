#include "transaction_server.h"
#include "coroutine_scheduler.h"
#include "transaction_mem.h"
#include "transaction_instance.h"

// std::unique_ptr<TransactionServer> g_trans_server_ptr;
TransactionServer* g_trans_server_ptr = nullptr;


TransactionServer::TransactionServer()
    : scheduler_ptr_{ new CoroutineScheduler{} }
    , tran_mgr_ptr_ { new TranInstMgr{} }
    , timer_mgr_ptr_ { new TimerMgr{} }
{
    LogDebug() << "TransactionServer::TransactionServer";
}

TransactionServer::~TransactionServer()
{
    LogDebug() << "TransactionServer::~TransactionServer";
}

s32 TransactionServer::Init()
{
    if (scheduler_ptr_ == nullptr)
    {
        LogError() << "scheduler_ptr_ is nullptr";
        return E_ERROR_INVALID_PARA;
    }
    scheduler_ptr_->Init();

    if (timer_mgr_ptr_ == nullptr)
    {
        LogError() << "timer_mgr_ptr_ is nullptr";
        return E_ERROR_INVALID_PARA;
    }
    timer_mgr_ptr_->Init();

    if (tran_mgr_ptr_ == nullptr)
    {
        LogError() << "tran_mgr_ptr_ is nullptr";
        return E_ERROR_INVALID_PARA;
    }

    // 注册定时器函数
    s32 ret = RegisterTimerFunc();
    if (ret != 0)
    {
        LogError() << "RegisterTimerFunc Faild, ret=" << ret;
        return ret;
    }

    return 0;
}

s32 TransactionServer::OnTick()
{
    timer_mgr()->Proc();
    
    return 0;
}

// 注册定时器函数
s32 TransactionServer::RegisterTimerFunc()
{
    s32 ret = 0;
    ret |= Timer::RegisterTimerFunc(E_BASE_TIMER_FUNC_ID_TRANSACTION_ON_TIMEOUT,
        TransactionInstance::TransactionOnTimeout);
    if (ret != 0)
    {
        LogError() << "register timer func error=" << ret;
        return -1;
    }
    return 0;
}

s32 TransactionServer::StartCommonTransaction(Transaction& transaction, u64 owner_id,
                               TransactionInstance** saved_inst, u64 param1, u64 param2) const
{
    if (owner_id == 0 || saved_inst == nullptr)
    {
        LogError() << "invalid arg";
        return E_ERROR_INVALID_PARA;
    }

    *saved_inst = nullptr;

    const s32 type = transaction.type();

    auto* inst = TransactionMem::CreateTransactionInst(transaction.type(), owner_id);
    if (inst == nullptr)
    {
        LogError() << "CreateTranInst failed, type=" << type;
        return E_ERROR_RESOURCE_UNAVAILABLE;
    }

    inst->SetParams(param1, param2);

    std::unique_ptr<TransactionInstance, void (*)(TransactionInstance*)> mem_guard {
        inst, &TransactionMem::DeleteTransactionInst
    };

    s32 ret = StartTransaction(inst);
    if (ret != 0)
    {
        LogError() << "StartTransaction failed, ret=" << ret;
        return ret;
    }

    *saved_inst = mem_guard.release();

    return 0;
}

s32 TransactionServer::StartTransaction(TransactionInstance *inst) const
{
    if (unlikely(inst == nullptr))
    {
        LogError() << "invalid arg";
        return E_ERROR_INVALID_PARA;
    }

    Transaction *transaction = GetTranByType(inst->type());
    if (!transaction)
    {
        LogError() << "GetTranByType="<< inst->type() << " failed";
        return E_ERROR_LOGIC;
    }

    s32 ret = tran_mgr_ptr_->AddTranInst(inst);
    if (ret != 0)
    {
        LogError() << "AddTranInst failed, ret=" << ret;
        return E_ERROR_SVR_INTERNAL;
    }

    ret = transaction->Start(inst);
    if (ret != 0)
    {
        LogError() << "TranInst.Start failed, ret=" << ret;
        tran_mgr_ptr_->RemoveTranInst(inst->id());
    }

    return ret;
}


s32 TransactionServer::SendMsgEventToTran(u64 tran_id, s32 event_id, void* data) const
{
    if (unlikely(tran_id == 0))
    {
        LogError() << "invalid arg";
        return E_ERROR_INVALID_PARA;
    }

    auto* inst = tran_mgr_ptr_->GetTranInst(tran_id);
    if (inst == nullptr)
    {
        LogError() << "GetTransaction failed, inst=" << tran_id;
        return E_ERROR_TRAN_INST_NOT_FOUND;
    }

    s32 ret = inst->SendMsgEvent(event_id, data);
    if (ret != 0)
    {
        LogError() << "SendMsgEvent failed, ret=" << ret;
        return E_ERROR_SVR_INTERNAL;
    }

    return 0;
}