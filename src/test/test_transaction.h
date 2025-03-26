#pragma once

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "singleton.h"
#include "proto_base.pb.h"
#include "common_def.h"
#include "boost_context.h"
#include "fstream"
#include "test_messages.pb.h"

#include "test_transaction_cmd.h"

int main(int argc, char *argv[])
{
    LogInfo() << "begin main call ...";

    // g_trans_server_ptr 全局只有一个
    g_trans_server_ptr = std::unique_ptr<TestTransactionServer>(new TestTransactionServer()); // C++11不支持make_unique...
    if (! g_trans_server_ptr)
    {
        LogFatal() << "new TestTransactionServer failed";
        return -1;
    }
    g_trans_server_ptr->Init();

    u64 owner = 123456789;
    TransactionInstance* tran_inst = nullptr;
    g_trans_server_ptr->StartCommonTransaction(
        EnterGameTransaction::Instance(),
        owner,
        &tran_inst
    );

    
    SSHead head;
    TestMessage msg;

    for (s32 _ = 0; _ < 10; ++ _)
    {
        for (s32 i = E_TRAN_EVENT_TYPE_MAX - 10; i <= E_TRAN_EVENT_TYPE_MAX; ++ i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            auto* ti = g_trans_server_ptr->tran_mgr()->GetTranInst(tran_inst->id());
            if (! ti)
            {
                break;
            }
            g_trans_server_ptr->SendMsgEventToTran(tran_inst->id(), i, head, msg);
        }
    }

    return 0;
}