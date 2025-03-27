#include "trans_server.h"
#include "trans.h"
#include "transaction_server.h"

s32 DemoTransactionServer::Init()
{
    s32 ret = TransactionServer::Init();
    if (ret != 0)
    {
        LogError() << "TransactionServer::Init failed";
        return ret;
    }

    // g_trans_server_ptr 全局只有一个
    auto* addr_server = &DemoTransactionServer::Instance();
    g_trans_server_ptr = addr_server;
    // g_trans_server_ptr = std::unique_ptr<DemoTransactionServer>(addr_server); // C++11不支持make_unique...

    return 0;
}

Transaction* DemoTransactionServer::GetTranByType(s32 type) const
{
    switch (type)
    {
    case E_TRANSACTION_TYPE_DEMO:
        return &DemoTransaction::Instance();

    default:
        LogError() << "unknown tran type=" << type;
        break;
    }

    return nullptr;
}