#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "singleton.h"
#include "common_def.h"
#include "fstream"
#include "trans_server.h"
#include "trans.h"
#include "transaction/transaction_server.h"


int main(int argc, char *argv[])
{
    // std::freopen(PROJECT_ROOT_DIR "/bin/log.txt", "a+", stdout);

    LogInfo() << "begin main call ...";

    DemoTransactionServer::Instance().Init();
   
    if (! g_trans_server_ptr)
    {
        LogFatal() << "new DemoTransactionServer failed";
        return -1;
    }

    u64 owner = 123456789;
    TransactionInstance* tran_inst = nullptr;
    g_trans_server_ptr->StartCommonTransaction(
        DemoTransaction::Instance(),
        owner,
        &tran_inst
    );


    u64 data = 100;

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
            g_trans_server_ptr->SendMsgEventToTran(tran_inst->id(), i, &data);
        }
    }


    LogInfo() << "end main call ...";
    return 0;
}