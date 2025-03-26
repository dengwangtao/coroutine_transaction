#include "transaction_mgr.h"

#include "coroutine_scheduler.h"
#include "error_define.pb.h"
#include "transaction.h"
#include "transaction_instance.h"
#include "transaction_server.h"

s32 TranInstMgr::AddTranInst(TransactionInstance* inst)
{
    if (!inst || !inst->id())
    {
        LogError() << "invalid args";
        return E_ERROR_INVALID_PARA;
    }

    auto result = trans_map_.emplace(inst->id(), inst);
    if (!result.second)
    {
        LogError() << "inster to map failed, id=" << inst->id();
        return E_ERROR_DUPLICATED_OPERATION;
    }

    return 0;
}

void TranInstMgr::Update()
{
    // 加入延迟处理，主要是因为没法在协程中销毁协程，必须等进程上下文切换到主协程以后，才能去销毁工作协程
    for (TransactionInstance *inst : destroy_list_)
    {
        assert(inst != nullptr);

        if (inst->coroutine_id() != 0)
        {
            s32 ret = g_coroutine_scheduler.DestroyWorkRoutine(inst->coroutine_id());
            if (ret != 0)
            {
                LogError() << "DestroyWorkRoutine failed, ret=" << ret;
            }
            inst->set_coroutine_id(0);
        }
        auto *tran = g_trans_server_ptr->GetTranByType(inst->type());
        if (!tran)
        {
            LogError() << _LogKV2("tran", inst->type(), inst->id()) << " unknow type";
        }
        else
        {
            tran->HandleResult(*inst);
        }

        inst->set_is_delay_destroying(false);
        inst->SafeRelease();
    }

    LogTrace() << "destroy trans size=" << destroy_list_.size();

    destroy_list_.clear();
}

s32 TranInstMgr::MarkTranInstDelayDestroy(TransactionInstance* inst)
{
    assert(inst != nullptr);

    LogTrace() << _LogKV2("tran inst", inst->type(), inst->id());

    if (inst->is_delay_destroying())
    {
        LogError() << _LogKV("inst", inst->id()) << " is already delay destroying";
        return E_ERROR_DUPLICATED_OPERATION;
    }

    inst->set_is_delay_destroying(true);

    if (!destroy_list_.insert(inst).second)
    {
        LogError() << _LogKV("inst", inst->id()) << "is already in destroy list";
    }

    if (trans_map_.erase(inst->id()) == 0)
    {
        LogError() << "find transaction failed, id=" << inst->id();
        return E_ERROR_LOGIC;
    }

    return 0;
}

s32 TranInstMgr::Resume()
{
    std::vector<TransactionInstance*> instances;
    instances.reserve(trans_map_.size());

    for (const auto &kvp : trans_map_)
    {
        instances.push_back(kvp.second);
    }

    for (TransactionInstance *inst : instances)
    {
        const s32 type = inst->type();
        const u64 id = inst->id();

        s32 ret = inst->Resume();
        if (ret != 0)
        {
            LogError() << _LogKV2("tran", type, id) << "resume failed";
        }
    }
    return 0;
}
