
#pragma once
#ifndef DEMO_TRANS_SERVER_H
#define DEMO_TRANS_SERVER_H

#include "transaction_server.h"
#include "trans_type.pb.h"
#include "TcpServer.h"

// =================== 事务服务器 ====================

class DemoTransactionServer : public TransactionServer, public Singleton<DemoTransactionServer>
{

public:

    s32 Init() override;

    Transaction *GetTranByType(s32 type) const override;

    s32 Start(u32 thread_num = 4);
    void OnConnection(const dwt::TcpConnectionPtr& conn);
    void OnMessage(const dwt::TcpConnectionPtr& conn, dwt::Buffer* buf, dwt::Timestamp time);

private:
    std::unique_ptr<dwt::TcpServer> tcp_server_;
    std::unique_ptr<dwt::EventLoop> event_loop_;
};


#endif // DEMO_TRANS_SERVER_H