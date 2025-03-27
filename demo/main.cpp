#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "common_def.h"
#include "trans_server.h"
#include "trans.h"
#include "transaction/transaction_server.h"
#include "gm.h"


int main(int argc, char *argv[])
{
    // std::freopen(PROJECT_ROOT_DIR "/bin/log.txt", "a+", stdout);

    RegisterAllGM(); // register all gm

    s32 ret = DemoTransactionServer::Instance().Init();
   
    if (ret)
    {
        LogFatal() << "DemoTransactionServer::Instance().Init() failed." << _LogK(ret);
        return ret;
    }

    auto& server = DemoTransactionServer::Instance();

    server.Start();
    return 0;
}