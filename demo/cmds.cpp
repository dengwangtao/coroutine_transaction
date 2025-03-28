#include "cmds.h"
#include "transaction/transaction_instance.h"
#include "transaction/transaction_server.h"

s32 TestCmd::Do(TransactionInstance &inst)
{
    LogInfo() << _LogKV("current_co_id", g_trans_server_ptr->co_scheduler()->curr_routine_id())
              << "TestCmd::Do" << _LogKV("inst", inst.id()) << _LogKV("inst_owner", inst.owner_id());
    return 0;
}

s32 TestCmd::OnRecvMsgEvent(TransactionInstance &inst, s32 event_id)
{
    LogInfo() << _LogKV("current_co_id", g_trans_server_ptr->co_scheduler()->curr_routine_id())
              << "TestCmd::OnRecvMsgEvent";
    return 0;
}

s32 TestCmd2::Do(TransactionInstance &inst)
{
    LogInfo() << _LogKV("current_co_id", g_trans_server_ptr->co_scheduler()->curr_routine_id())
              << "TestCmd2::Do" << _LogKV("inst", inst.id()) << _LogKV("inst_owner", inst.owner_id());
    return 0;
}

s32 TestCmd2::OnRecvMsgEvent(TransactionInstance &inst, s32 event_id)
{
    LogInfo() << _LogKV("current_co_id", g_trans_server_ptr->co_scheduler()->curr_routine_id())
              << "TestCmd2::OnRecvMsgEvent";
    return 0;
}

s32 TestCmd3::Do(TransactionInstance &inst)
{
    LogInfo() << _LogKV("current_co_id", g_trans_server_ptr->co_scheduler()->curr_routine_id())
              << "TestCmd3::Do" << _LogKV("inst", inst.id()) << _LogKV("inst_owner", inst.owner_id());
    return 0;
}

s32 TestCmd3::OnRecvMsgEvent(TransactionInstance &inst, s32 event_id)
{
    LogInfo() << _LogKV("current_co_id", g_trans_server_ptr->co_scheduler()->curr_routine_id())
              << "TestCmd3::OnRecvMsgEvent";
    return 0;
}