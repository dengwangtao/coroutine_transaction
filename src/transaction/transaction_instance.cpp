#include "transaction_instance.h"

void TransactionInstance::TransactionOnTimeout(
    u64 timer_id, void* data, size_t data_len)
{
    if (NULL == data || data_len < sizeof(u64))
    {
        error_tlog("invalid args");
        return;
    }

    u64 tran_id = *(reinterpret_cast<u64*>(data));
    TransactionInstance* inst = g_shm_svr->tran_mgr()->GetTranInst(tran_id);
    if (NULL == inst)
    {
        error_tlog("TranMgrInst.GetTranInst=%lu failed", tran_id);
        return;
    }

    s32 ret = inst->SendMsgEvent(E_TRANSACTION_EVENT_TYPE_TIMEOUT, NULL);
    if (ret != 0)
    {
        error_tlog("SendMsgEvent failed, ret=%d", ret);
    }
}