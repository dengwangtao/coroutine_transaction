#include "cmds.h"
#include "transaction/transaction_instance.h"

s32 TestCmd::Do(TransactionInstance &inst)
{
    LogInfo() << "TestCmd::Do" << _LogKV("inst", inst.id()) << _LogKV("inst_owner", inst.owner_id());
    return 0;
}

s32 TestCmd::OnRecvMsgEvent(TransactionInstance &inst, s32 event_id)
{
    LogInfo() << "TestCmd::OnRecvMsgEvent";
    return 0;
}

s32 TestCmd2::Do(TransactionInstance &inst)
{
    LogInfo() << "TestCmd2::Do" << _LogKV("inst", inst.id()) << _LogKV("inst_owner", inst.owner_id());
    return 0;
}

s32 TestCmd2::OnRecvMsgEvent(TransactionInstance &inst, s32 event_id)
{
    LogInfo() << "TestCmd2::OnRecvMsgEvent";
    return 0;
}

s32 TestCmd3::Do(TransactionInstance &inst)
{
    LogInfo() << "TestCmd3::Do" << _LogKV("inst", inst.id()) << _LogKV("inst_owner", inst.owner_id());
    return 0;
}

s32 TestCmd3::OnRecvMsgEvent(TransactionInstance &inst, s32 event_id)
{
    LogInfo() << "TestCmd3::OnRecvMsgEvent";
    return 0;
}