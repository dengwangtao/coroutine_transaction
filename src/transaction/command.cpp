#include "command.h"

#include "error_define.pb.h"
#include "transaction.h"
#include "transaction_instance.h"



#define TranLog(LOG_LEVEL) \
    Log##LOG_LEVEL() << "[" << GetName() << "] "


s32 Command::DoAndWait(TransactionInstance &inst)
{
    s32 ret = Do(inst);
    if (ret != 0)
    {
        TranLog(Error) << _LogKV2("trans", inst.type(), inst.id()) << " Do failed, " << _LogK(ret);
        return ret;
    }

    if (!inst.should_wait_current_cmd())
    {
        TranLog(Error) << _LogKV2("trans", inst.type(), inst.id()) << " no need to wait ";
        return 0;
    }

    ret = Wait(inst);
    if (ret != 0)
    {
        TranLog(Error) << _LogKV2("trans", inst.type(), inst.id()) << " Wait failed, " << _LogK(ret);
    }

    return ret;
}

s32 Command::WaitForEvent(TransactionInstance& inst, const EventIdVec& msg_event_id_vec)
{
    EventIdVec events = msg_event_id_vec;

    while(!events.empty())
    {
        s32 ret = inst.Wait(events.data(), events.size(), timeout_ms_);
        if (ret != 0)
        {
            LogError() << "Wait failed, ret=" << ret;
            return E_ERROR_SVR_INTERNAL;
        }

        TranLog(Trace)  << _LogKV("uid", inst.owner_id()) << _LogKV2("tran", inst.type(), inst.id())
                        << _LogKV("event", inst.event_type());

        auto it = std::find(events.begin(), events.end(), inst.event_type());
        if (it != events.end())
        {
            ret = this->OnRecvMsgEvent(inst, inst.event_type());
            if (ret != 0)
            {
                TranLog(Error) << "OnRecvMsgEvent failed, ret=" << ret;
                return ret;
            }
            // 继续等待其他事件
            events.erase(it);
            continue;
        }

        bool is_proc = false;
        ret = inst.ProcDefaultEvents(is_proc);
        if (ret != 0)
        {
            TranLog(Error) << "ProcDefaultEvents failed, ret=" << ret;
            return ret;
        }
    }

    return 0;
}

s32 Command::Wait(TransactionInstance &inst)
{
    if (do_event_id_vec_.empty())
    {
        TranLog(Error) << "event_id not set";
        return E_ERROR_LOGIC;
    }

    s32 ret = WaitForEvent(inst, do_event_id_vec_);
    if (ret != 0)
    {
        TranLog(Error) << "WaitForEvent failed, ret=" << ret;
        return ret;
    }

    return 0;
}

const char* Command::GetName() const
{
    // 只有第一次预测失败
    if (likely(demangled_cls_name_[0] != '\0'))
    {
        return demangled_cls_name_;
    }

    // Command之前没提供Init函数, 如果加了需要大量修改老代码.
    (void) CommonUtil::GetDemangledName(
        this, demangled_cls_name_, sizeof(demangled_cls_name_));

    return demangled_cls_name_;
}

s32 Command::OnTimeout(TransactionInstance &inst)
{
    TranLog(Error) << _LogKV2("tran_inst", inst.type(), inst.id()) << " timeout";
    return E_ERROR_TIMEOUT;
}

void Command::Undo(TransactionInstance& inst)
{
    inst.set_should_wait_current_cmd(false);
}
