#pragma once
#ifndef DEMO_CMDS_H_
#define DEMO_CMDS_H_

#include "transaction/command.h"
#include "proto/trans_type.pb.h"


class TestCmd : public Command
{
public:
    TestCmd() : Command(E_TRAN_EVENT_TYPE_DEMO)
    {
    }
    s32 Do(TransactionInstance &inst) override;
    
    s32 OnRecvMsgEvent(TransactionInstance &inst, s32 event_id) override;
};

class TestCmd2 : public Command
{
public:
    TestCmd2() : Command(E_TRAN_EVENT_TYPE_DEMO_2)
    {
    }
    s32 Do(TransactionInstance &inst) override;
    
    s32 OnRecvMsgEvent(TransactionInstance &inst, s32 event_id) override;
};

class TestCmd3 : public Command
{
public:
    TestCmd3() : Command(E_TRAN_EVENT_TYPE_DEMO_3)
    {
    }
    s32 Do(TransactionInstance &inst) override;
    
    s32 OnRecvMsgEvent(TransactionInstance &inst, s32 event_id) override;
};


#endif // DEMO_CMDS_H_