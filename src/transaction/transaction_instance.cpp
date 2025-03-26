#include "transaction_instance.h"
#include "transaction_server.h"
#include "transaction/transaction_mem.h"
#include "gen_guid.h"


#define TranInstLog(LOG_LEVEL)                      \
    Log##LOG_LEVEL() << _LogKV2("tran", type_, id_) \
                     << _LogKV("owner", owner_id_) << " "


void TransactionInstance::TransactionOnTimeout(
    u64 timer_id, void *data, size_t data_len)
{
    if (NULL == data || data_len < sizeof(u64))
    {
        LogError() << "invalid args";
        return;
    }

    u64 tran_id = *(reinterpret_cast<u64 *>(data));
    TransactionInstance *inst = g_trans_server_ptr->tran_mgr()->GetTranInst(tran_id);
    if (NULL == inst)
    {
        LogError() << "TranMgrInst.GetTranInst=" << tran_id << " failed";
        return;
    }

    s32 ret = inst->SendMsgEvent(E_TRANSACTION_EVENT_TYPE_TIMEOUT, NULL);
    if (ret != 0)
    {
        LogError() << "SendMsgEvent failed, ret=" << ret;
    }
}

TransactionInstance::TransactionInstance(s32 type, u64 owner_id)
    : id_{ GenGUID() },
      owner_id_{owner_id},
      type_{type}
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
    DeleteTransactionInst(this);
}

void TransactionInstance::SafeRelease()
{
    if (is_delay_destroying_)
    {
        TranInstLog(Error) << " is delay destroying, cannot release directly";
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
        LogError() << "invalid args";
        return E_ERROR_INVALID_PARA;
    }

    TranInstLog(Trace) << " msg=" << msg_type;

    if (msg_type == E_TRANSACTION_EVENT_TYPE_TIMEOUT)
    {
        timer_id_ = INVALID_TIMER_ID;
    }
    else if (timer_id_ != INVALID_TIMER_ID)
    {
        s32 ret = g_trans_server_ptr->timer_mgr()->DestroyTimer(timer_id_);
        if (ret != 0)
        {
            LogError() << "destroy transaction timer=" << timer_id_ << " failed";
        }
        timer_id_ = INVALID_TIMER_ID;
    }

    bool is_default_event = IsDefaultEvent(msg_type);
    if (!is_default_event)
    {
        s32 *event = std::find(events_, events_ + event_count_, msg_type);
        if (event == events_ + event_count_)
        {
            TranInstLog(Error) << "not wait for msg=" << msg_type;
            for (s32 i = 0; i < event_count_; ++i)
            {
                LogError() << "waiting msg[" << i << "]: " << events_[i];
            }
            return E_ERROR_LOGIC;
        }
    }

    SetEventArg(msg_type, head, msg);
    s32 ret = g_trans_server_ptr->co_scheduler()->SwapToWorkRoutine(coroutine_id_);
    if (ret != 0)
    {
        LogError() << "SwapToWorkRoutine failed, coroutine id=" << coroutine_id_;
        return ret;
    }

    return 0;
}

s32 TransactionInstance::SendMsgEvent(s32 msg_type, void *data)
{
    if (msg_type == E_TRANSACTION_EVENT_TYPE_INVALID)
    {
        LogError() << "invalid args";
        return E_ERROR_INVALID_PARA;
    }

    TranInstLog(Trace) << " msg=" << msg_type;

    if (msg_type == E_TRANSACTION_EVENT_TYPE_TIMEOUT)
    {
        timer_id_ = INVALID_TIMER_ID;
    }
    else if (timer_id_ != INVALID_TIMER_ID)
    {
        s32 ret = g_trans_server_ptr->timer_mgr()->DestroyTimer(timer_id_);
        if (ret != 0)
        {
            LogError() << "destroy transaction timer=" << timer_id_ << " failed";
        }
        timer_id_ = INVALID_TIMER_ID;
    }

    bool is_default_event = IsDefaultEvent(msg_type);
    if (!is_default_event)
    {
        s32 *event = std::find(events_, events_ + event_count_, msg_type);
        if (event == events_ + event_count_)
        {
            TranInstLog(Error) << "not wait for msg=" << msg_type;
            for (s32 i = 0; i < event_count_; ++i)
            {
                LogError() << "waiting msg[" << i << "]: " << events_[i];
            }
            return E_ERROR_LOGIC;
        }
    }

    SetEventArg(msg_type, data);
    s32 ret = g_trans_server_ptr->co_scheduler()->SwapToWorkRoutine(coroutine_id_);
    if (ret != 0)
    {
        LogError() << "SwapToWorkRoutine failed, coroutine id=" << coroutine_id_;
        return ret;
    }

    return 0;
}

s32 TransactionInstance::Wait(s32 *events, s32 event_count, s32 timeout_ms)
{
    if (NULL == events || event_count <= 0 || timeout_ms <= 0 || event_count > CommonUtil::array_size(events_))
    {
        LogError() << "invlaid args";
        return E_ERROR_INVALID_PARA;
    }

    do
    {

        if (timer_id_ != INVALID_TIMER_ID)
        {
            break;
        }

        u64 timer_id = g_trans_server_ptr->timer_mgr()->RegisterTimer(timeout_ms, 1,
                                                                      E_BASE_TIMER_FUNC_ID_TRANSACTION_ON_TIMEOUT, const_cast<u64 *>(&id_));
        if (INVALID_TIMER_ID == timer_id)
        {
            TranInstLog(Error) << "RegisterTimer failed";
            return E_ERROR_RESOURCE_UNAVAILABLE;
        }

        timer_id_ = timer_id;
        waiting_index_ = curr_index_;

        memcpy(events_, events, sizeof(s32) * event_count);
        event_count_ = event_count;
    } while (0);

    TranInstLog(Trace) << "waiting_index=" << waiting_index_;

    s32 ret = g_trans_server_ptr->co_scheduler()->SwapToMain();
    if (ret != 0)
    {
        LogError() << "SwapToMain failed, ret=" << ret;
        return ret;
    }

    return 0;
}

s32 TransactionInstance::Abort()
{
    TranInstLog(Trace) << "abort";

    s32 ret = SendMsgEvent(E_TRANSACTION_EVENT_TYPE_ABORT, NULL);
    if (ret != 0)
    {
        TranInstLog(Error) << "SendMsgEvent failed, ret=" << ret;
        return ret;
    }
    return 0;
}

s32 TransactionInstance::ProcDefaultEvents(bool &is_proc)
{
    if (event_type() == E_TRANSACTION_EVENT_TYPE_ABORT)
    {
        LogInfo() << "recv abort event," << _LogKV2("tran_inst", type(), id_);
        is_proc = true;
        set_fail_reason(E_ERROR_ABORT_TRAN);
    }
    else if (event_type() == E_TRANSACTION_EVENT_TYPE_TIMEOUT)
    {
        Transaction *tran = g_trans_server_ptr->GetTranByType(type_);
        if (unlikely(!tran))
        {
            LogError() << "GetTranByType=" << type_ << " failed";
            return E_ERROR_INVALID_PARA;
        }
        if (unlikely(curr_index_ < 0 || curr_index_ >= tran->cmd_array_.size()))
        {
            LogError() << "Invalid " << _LogKV("curr_index", curr_index_)
                       << _LogKV("cmd_array_size", tran->cmd_array_.size());
            return E_ERROR_INVALID_PARA;
        }
        Command *cmd = tran->cmd_array_[curr_index_];
        if (unlikely(!cmd))
        {
            LogError() << _LogKV("tran_inst", id_) << "get cmd index=" << curr_index_ << " failed";
            return E_ERROR_INVALID_PARA;
        }
        is_proc = true;
        return cmd->OnTimeout(*this);
    }

    return 0;
}

void TransactionInstance::SetEventArg(s32 type, void *msg)
{
    event_type_ = type;
    event_arg_.msg = msg;
}

void TransactionInstance::SetEventArg(s32 type, const SSHead &head,
                                      const google::protobuf::Message &body)
{
    event_type_ = type;
    event_arg_.msg = TransactionEventArg::ServerProtoMessage{&head, &body};
}

s32 TransactionInstance::Resume()
{
    Transaction *tran = g_trans_server_ptr->GetTranByType(type_);
    if (tran == NULL)
    {
        LogError() << _LogKV("GetTranByType", type_) << " failed, inst=" << id_;
        return E_ERROR_SVR_INTERNAL;
    }

    s32 ret = tran->Resume(*this);
    if (ret != 0)
    {
        LogError() << "resumt" << _LogKV2("tran", tran->GetName(), id_) << "failed, ret=" << ret;
        return ret;
    }

    return 0;
}
