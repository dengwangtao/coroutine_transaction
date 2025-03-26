#include "transaction.h"
#include <string.h>

#include "common_def.h"
#include "transaction.h"
#include "command.h"
#include "coroutine_def.h"
#include "transaction_server.h"
#include "transaction_instance.h"
#include "transaction/transaction_mgr.h"
#include "cs_req_id_util.h"



#define TranLog(LOG_LEVEL) \
    Log##LOG_LEVEL() << "[" << GetName() << "] "

Transaction::Transaction(s32 type, bool is_need_undo)
    : type_ { type },
      is_need_undo_ { is_need_undo }
{
    demangled_cls_name_[0] = '\0';
}

Transaction::~Transaction()
{
}

const char* Transaction::GetName() const
{
    // 只有第一次预测失败
    if (likely(demangled_cls_name_[0] != '\0'))
    {
        return demangled_cls_name_;
    }

    (void) CommonUtil::GetDemangledName(this, demangled_cls_name_, sizeof(demangled_cls_name_));

    return demangled_cls_name_;
}

const char* Transaction::GetCmdName(int index) const
{
    if (unlikely(index < 0 || index >= cmd_array_.size()))
    {
        return "";
    }

    return cmd_array_[index]->GetName();
}

s32 Transaction::Start(TransactionInstance *inst)
{
    if (!inst)
    {
        return E_ERROR_INVALID_PARA;
    }

    if (inst->coroutine_id() != 0)
    {
        TranLog(Error) << "already started, curr_index=" << inst->curr_index();
        return E_ERROR_LOGIC;
    }

    (void) cs_req_id_util::SetTranInst(*inst);
    

    u64 coroutine_id = 0;
    s32 ret = g_trans_server_ptr->co_scheduler()->CreateWorkRoutine(
            inst->stack_size(),
            Transaction::TransactionCoroutineBootEntry,
            (void*)inst->id(),
            coroutine_id
    );

    if (ret != 0)
    {
        LogError() << "Transaction<" << GetName() << ">CreateWorkRoutine failed, ret="<< ret;
        return ret;
    }

    inst->set_coroutine_id(coroutine_id);

    ret = g_trans_server_ptr->co_scheduler()->SwapToWorkRoutine(coroutine_id);
    if (ret != 0)
    {
        inst->set_coroutine_id(0);

        (void) g_trans_server_ptr->co_scheduler()->DestroyWorkRoutine(coroutine_id);
        TranLog(Error) << "SwapToWorkRoutine failed, ret=" << ret;
        return ret;
    }

    return 0;
}

void Transaction::RunCommandOnInstance(TransactionInstance &inst, int index)
{
    for (int i = index; i < cmd_array_.size(); ++i)
    {
        inst.set_curr_index(i);

        Command* command = cmd_array_[i];

        TranLog(Debug) << _LogKV2("inst", inst.id(), command->GetName()) << "::Do owner=" << inst.owner_id();

        inst.set_should_wait_current_cmd(true);
        s32 error_code = command->DoAndWait(inst);

        if (error_code != 0 || inst.IsFailed())
        {
            TranLog(Debug)  << _LogKV2("inst", inst.id(), command->GetName()) << "::Do() failed"
                            << _LogK(index) << _LogKV2("ret", i, error_code);

            inst.set_fail_index(i);
            if (inst.fail_reason() == 0)
            {
                inst.set_fail_reason(error_code);
            }
            break;
        }
        else if (inst.is_complete())
        {
            TranLog(Info) << _LogKV2("inst", inst.id(), command->GetName()) << " complete at "
                          << i << "/" << cmd_array_.size();
            break;
        }
    }
}

s32 Transaction::RealStart(TransactionInstance& inst)
{
    const u64 tran_id = inst.id();
    TranLog(Debug) << _LogKV("inst", tran_id) << _LogKV("owner", inst.owner_id());

    int ret = OnStart(inst);
    if (ret != 0)
    {
        TranLog(Error) << "OnStart failed, ret=" << ret;
        inst.set_fail_reason(E_ERROR_SVR_INTERNAL);
    }
    else
    {
        RunCommandOnInstance(inst, 0);
    }

    if (!inst.IsFailed() && !inst.is_complete())
    {
        inst.complete();
        TranLog(Info) << _LogKV("id", tran_id) << " succeeded" << _LogKV("owner", inst.owner_id());
    }

    ret = g_trans_server_ptr->tran_mgr()->MarkTranInstDelayDestroy(&inst);
    if (ret != 0)
    {
        TranLog(Error) << "MarkTranInstDelayDestroy failed, inst=" << tran_id;
    }

    return 0;
}

void Transaction::HandleResult(TransactionInstance &inst)
{
    if (g_trans_server_ptr->co_scheduler()->IsInCoroutine())
    {
        TranLog(Error) << "not in main routine, inst=" << inst.id();
        return;
    }


    auto cris = cs_req_id_util::Push(inst.cs_req_id());

    if (inst.is_complete())
    {
        (void) OnSuccess(inst);
    }
    else
    {
        s32 ret = 0;
        if (inst.fail_reason() == E_ERROR_ABORT_TRAN)
        {
            ret = OnAbort(inst);
        }
        else
        {
            ret = OnFail(inst);
        }

        if (is_need_undo_)
        {
            ret = Undo(inst);
        }

    }

    Finally(inst);

    cs_req_id_util::Pop(cris);
}

s32 Transaction::Undo(TransactionInstance& inst)
{
    if (!is_need_undo_)
    {
        return 0;
    }

    // It's possible that crash occurs during undo.
    if (inst.is_in_undo())
    {
        TranLog(Error) << _LogKV("inst", inst.id()) << _LogKV("fail_idx", inst.fail_index()) << " already in undo";
        return E_ERROR_ALREADY_IN_PROGRESS;
    }

    s32 undo_idx = inst.fail_index();
    if (undo_idx < 0)
    {
        TranLog(Error) << _LogKV("inst", inst.id()) << "fail idx not set, may crash in do=" << inst.curr_index();
        return E_ERROR_UNKNOWN;
    }

    // Undo from the index before the failed one since we don't know if the
    // failed command can be reverted.
    if (undo_idx == 0)
    {
        return 0;
    }

    if (undo_idx >= cmd_array_.size())
    {
        TranLog(Error)  << _LogKV("inst", inst.id()) << _LogKV("undo idx", undo_idx)
                        << " is larger than command size=" << cmd_array_.size();    
        return E_ERROR_SVR_INTERNAL;
    }

    inst.set_is_in_undo(true);
    for (s32 i = undo_idx - 1; i >= 0; --i)
    {
        // 回滚的时候不中断，尝试尽量回滚每一步
        TranLog(Info)   << _LogKV("inst", inst.id()) << _LogKV("idx", i)
                        << _LogKV("cmd", cmd_array_[i]->GetName())
                        << _LogKV("owner", inst.owner_id());
        cmd_array_[i]->Undo(inst);
    }
    inst.set_fail_index(-1);
    inst.set_is_in_undo(false);

    return 0;
}

s32 Transaction::Resume(TransactionInstance& inst)
{
    u64 co_id = inst.coroutine_id();

    s32 ret = g_trans_server_ptr->co_scheduler()->RestartCoroutine(
        co_id, Transaction::TransactionCoroutineResumeEntry, (void*)inst.id());
    if (ret == 0)
    {
        ret = g_trans_server_ptr->co_scheduler()->SwapToWorkRoutine(co_id);
        if (ret == 0)
        {
            return 0;
        }

        TranLog(Error) << _LogKV("inst", inst.id()) << "SwapToWorkRountine=" << co_id << " failed, ret=" << ret;
    }

    TranLog(Error) << _LogKV("inst", inst.id()) << "restart coroutine=" << co_id << " failed";

    inst.set_fail_index(inst.curr_index());
    inst.set_fail_reason(ret);

    const u64 tran_id = inst.id();

    ret = g_trans_server_ptr->tran_mgr()->MarkTranInstDelayDestroy(&inst);
    if (ret != 0)
    {
        TranLog(Error) << "MarkTranInstDelayDestroy=" << tran_id << " failed, ret=" << ret;
    }

    return E_ERROR_SVR_INTERNAL;
}

s32 Transaction::RealResume(TransactionInstance &inst)
{
    u64 tran_id = inst.id();
    
    TranLog(Info)   << _LogKV("inst", tran_id) << _LogKV("owner", inst.owner_id())
                    << _LogKV("curr_index", inst.curr_index())
                    << _LogKV("waiting_index", inst.waiting_index())
                    << _LogKV("fail_index", inst.fail_index());

    do
    {
        // Crash may happen during Command::Do of curr_index
        s32 waiting_index = inst.waiting_index();
        if (waiting_index != inst.curr_index())
        {
            // inst.curr_index - 1 maybe better
            inst.set_fail_index(inst.curr_index());
            break;
        }

        if (waiting_index >= cmd_array_.size())
        {
            TranLog(Error) << _LogKV("inst", tran_id) << " invalid waiting index=" << waiting_index;
            break;
        }

        if (waiting_index >= 0)
        {
            s32 error_code = cmd_array_[waiting_index]->Wait(inst);
            if (error_code != 0)
            {
                inst.set_fail_index(waiting_index);
                inst.set_fail_reason(error_code);
                break;
            }
        }

        RunCommandOnInstance(inst, inst.curr_index() + 1);
    } while (0);

    if (!inst.IsFailed() && !inst.is_complete())
    {
        inst.complete();
        TranLog(Info) << _LogKV("inst", tran_id) << " succeeded." << _LogKV("owner", inst.owner_id());
    }

    s32 ret = g_trans_server_ptr->tran_mgr()->MarkTranInstDelayDestroy(&inst);
    if (ret != 0)
    {
        error_tlog_tran("MarkTranInstDelayDestroy=%lu failed, ret=%d", tran_id, ret);
    }

    return 0;
}

void Transaction::TransactionCoroutineEntry(void* param, bool is_resume)
{
    static BoostContext curr_context = NULL;
    do
    {
        if (NULL == param)
        {
            error_tlog("invalid args");
            break;
        }

        u64 tran_id = reinterpret_cast<u64>(param);
        TransactionInstance* inst = g_shm_svr->tran_mgr()->GetTranInst(tran_id);
        if (NULL == inst)
        {
            error_tlog("GetTranInst=%lu failed", tran_id);
            break;
        }

        Transaction* transaction = g_shm_svr->GetTranByType(inst->type());
        if (NULL == transaction)
        {
            error_tlog("GetTranByType=%d failed, inst=%lu", inst->type(), tran_id);
            break;
        }

        s32 ret = 0;
        if (!is_resume)
        {
            ret = transaction->RealStart(*inst);
        }
        else
        {
            ret = transaction->RealResume(*inst);
        }
        if (ret != 0)
        {
            error_tlog("transaction proc failed, ret=%d tran=%lu", ret, tran_id);
        }

        ret = g_trans_server_ptr->co_scheduler()->OnWorkRoutineExit();
        if (ret != 0)
        {
            error_tlog("CoSchedulerInst.OnWorkRoutineExit failed, ret=%d.", ret);
        }
    } while (0);

    // boost context没有uclink, 必须自己跳转回主协程
    JumpContext(&curr_context,
        g_trans_server_ptr->co_scheduler()->main_routine().context(), NULL, false);
}

void Transaction::TransactionCoroutineBootEntry(void* param)
{
    TransactionCoroutineEntry(param, false);
}

void Transaction::TransactionCoroutineResumeEntry(void* param)
{
    TransactionCoroutineEntry(param, true);
}

TransactionInstance::TransactionInstance(s32 type, u64 owner_id)
    : id_ { GuidGen::Instance().GenLocalGuid(E_GUID_TYPE_TRAN) },
      owner_id_ { owner_id },
      type_ { type }
{
    timer_id_ = INVALID_TIMER_ID;
    coroutine_id_ = 0;
    event_count_ = 0;
    memset(events_, 0, sizeof(events_));
    event_type_ = 0;
    fail_reason_ = 0;
    is_in_undo_ = false;
    stack_size_ = kDefaultStackSize;
    param1_ = 0;
    param2_ = 0;
}

TransactionInstance::~TransactionInstance()
{
}

void TransactionInstance::Release()
{
    FrameShmObjCreate::DeleteTransactionInst(this);
}

void TransactionInstance::SafeRelease()
{
    if (is_delay_destroying_)
    {
        error_tlog_inst("is delay destroying, cannot release directly");
    }
    else
    {
        Release();
    }
}

s32 TransactionInstance::SendMsgEvent(s32 msg_type, const SSHead &head,
                                      const google::protobuf::Message &msg)
{
    if (msg_type == E_TRANSACTION_EVENT_TYPE_INVALID)
    {
        error_tlog("invalid args");
        return E_WX_ERROR_INVALID_PARA;
    }

    trace_tlog_inst("msg=%d", msg_type);

    if (msg_type == E_TRANSACTION_EVENT_TYPE_TIMEOUT)
    {
        timer_id_ = INVALID_TIMER_ID;
    }
    else if (timer_id_ != INVALID_TIMER_ID)
    {
        s32 ret = g_shm_svr->time_mgr()->DestroyTimer(timer_id_);
        if (ret != 0)
        {
            error_tlog("destroy transaction timer=%lu failed", timer_id_);
        }
        timer_id_ = INVALID_TIMER_ID;
    }

    bool is_default_event = IsDefaultEvent(msg_type);
    if (!is_default_event)
    {
        s32 *event = std::find(events_, events_ + event_count_, msg_type);
        if (event == events_ + event_count_)
        {
            error_tlog_inst("not wait for msg =%d", msg_type);
            for (s32 i = 0; i < event_count_; ++i)
            {
                error_tlog("waiting msg[%d]: %d", i, events_[i]);
            }
            return E_ERROR_LOGIC;
        }
    }

    SetEventArg(msg_type, head, msg);
    s32 ret = g_trans_server_ptr->co_scheduler()->SwapToWorkRoutine(coroutine_id_);
    if (ret != 0)
    {
        error_tlog("SwapToWorkRoutine failed, coroutine id=%lu.", coroutine_id_);
        return ret;
    }

    return 0;
}

s32 TransactionInstance::SendMsgEvent(s32 msg_type, void *data)
{
    if (msg_type == E_TRANSACTION_EVENT_TYPE_INVALID)
    {
        error_tlog("invalid args");
        return E_WX_ERROR_INVALID_PARA;
    }

    trace_tlog_inst("msg=%d", msg_type);

    if (msg_type == E_TRANSACTION_EVENT_TYPE_TIMEOUT)
    {
        timer_id_ = INVALID_TIMER_ID;
    }
    else if (timer_id_ != INVALID_TIMER_ID)
    {
        s32 ret = g_shm_svr->time_mgr()->DestroyTimer(timer_id_);
        if (ret != 0)
        {
            error_tlog("destroy transaction timer=%lu failed", timer_id_);
        }
        timer_id_ = INVALID_TIMER_ID;
    }

    bool is_default_event = IsDefaultEvent(msg_type);
    if (!is_default_event)
    {
        s32 *event = std::find(events_, events_ + event_count_, msg_type);
        if (event == events_ + event_count_)
        {
            error_tlog_inst("not wait for msg =%d", msg_type);
            for (s32 i = 0; i < event_count_; ++i)
            {
                error_tlog("waiting msg[%d]: %d", i, events_[i]);
            }
            return E_ERROR_LOGIC;
        }
    }

    SetEventArg(msg_type, data);
    s32 ret = g_trans_server_ptr->co_scheduler()->SwapToWorkRoutine(coroutine_id_);
    if (ret != 0)
    {
        error_tlog("SwapToWorkRoutine failed, coroutine id=%lu.", coroutine_id_);
        return ret;
    }

    return 0;
}

s32 TransactionInstance::Wait(s32* events, s32 event_count, s32 timeout_ms)
{
    if (NULL == events || event_count <= 0 || timeout_ms <= 0 || event_count > array_size(events_))
    {
        error_tlog("invlaid args");
        return E_WX_ERROR_INVALID_PARA;
    }

    do
    {

        if (timer_id_ != INVALID_TIMER_ID)
        {
            break;
        }

        u64 timer_id = g_shm_svr->time_mgr()->RegisterTimer(timeout_ms, 1,
            E_BASE_TIMER_FUNC_ID_TRANSACTION_ON_TIMEOUT, const_cast<u64*>(&id_));
        if (INVALID_TIMER_ID == timer_id)
        {
            error_tlog_inst("RegisterTimer failed");
            return E_WX_ERROR_RESOURCE_UNAVAILABLE;
        }

        timer_id_ = timer_id;
        waiting_index_ = curr_index_;

        memcpy(events_, events, sizeof(s32) * event_count);
        event_count_ = event_count;
    }
    while (0);

    trace_tlog_inst("waiting_index=%d", waiting_index_);

    s32 ret = g_trans_server_ptr->co_scheduler()->SwapToMain();
    if (ret != 0)
    {
        error_tlog("SwapToMain failed, ret=%d.", ret);
        return ret;
    }

    return 0;
}

s32 TransactionInstance::Abort()
{
    infor_tlog_inst("abort");

    s32 ret = SendMsgEvent(E_TRANSACTION_EVENT_TYPE_ABORT, NULL);
    if (ret != 0)
    {
        error_tlog_inst("SendMsgEvent failed, ret=%d", ret);
        return ret;
    }
    return 0;
}

s32 TransactionInstance::ProcDefaultEvents(bool& is_proc)
{
    if (event_type() == E_TRANSACTION_EVENT_TYPE_ABORT)
    {
        infor_tlog("recv abort event, tran_inst=%d:%lu.", type(), id_);
        is_proc = true;
        set_fail_reason(E_WX_ERROR_ABORT_TRAN);
    }
    else if (event_type() == E_TRANSACTION_EVENT_TYPE_TIMEOUT)
    {
        Transaction *tran = g_shm_svr->GetTranByType(type_);
        if (unlikely(!tran))
        {
            error_tlog("GetTranByType=%d failed", type_);
            return E_WX_ERROR_INVALID_PARA;
        }
        if (unlikely(curr_index_ < 0 || curr_index_ >= tran->cmd_array_.size()))
        {
            error_tlog("invalid curr_index_=%u, cmd_array_size=%u", curr_index_, tran->cmd_array_.size());
            return E_WX_ERROR_INVALID_PARA;
        }
        Command *cmd = tran->cmd_array_[curr_index_];
        if (unlikely(!cmd))
        {
            error_tlog("tran_inst=%lu get cmd index=%d failed", id_, curr_index_);
            return E_WX_ERROR_INVALID_PARA;
        }
        is_proc = true;
        return cmd->OnTimeout(*this);
    }

    return 0;
}

void TransactionInstance::SetEventArg(s32 type, void* msg)
{
    event_type_ = type;
    event_arg_.msg = msg;
}

void TransactionInstance::SetEventArg(s32 type, const SSHead &head,
                                      const google::protobuf::Message &body)
{
    event_type_ = type;
    event_arg_.msg = TransactionEventArg::ServerProtoMessage { &head, &body };
}

s32 TransactionInstance::Resume()
{
    Transaction* tran = g_shm_svr->GetTranByType(type_);
    if (tran == NULL)
    {
        error_tlog("GetTranByType=%d failed, inst=%lu", type_, id_);
        return E_WX_ERROR_SVR_INTERNAL;
    }

    s32 ret = tran->Resume(*this);
    if (ret != 0)
    {
        error_tlog("resume tran=%s:%lu failed, ret=%d", tran->GetName(), id_, ret);
        return ret;
    }

    return 0;
}
