#include "transaction_instance.h"
#include "transaction_mgr.h"

void TransactionInstance::TransactionOnTimeout(
    u64 timer_id, void* data, size_t data_len)
{
    if (NULL == data || data_len < sizeof(u64))
    {
        LogError() << "invalid args";
        return;
    }

    u64 tran_id = *(reinterpret_cast<u64*>(data));
    TransactionInstance* inst = g_tran_inst_mgr.GetTranInst(tran_id);
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