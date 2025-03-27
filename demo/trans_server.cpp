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

    dwt::InetAddress addr("127.0.0.1", 8080);
    event_loop_ = std::make_unique<dwt::EventLoop>();
    if (event_loop_ == nullptr)
    {
        LogError() << "make event_loop failed";
        return -1;
    }

    tcp_server_ = std::make_unique<dwt::TcpServer>(event_loop_.get(), addr, "DemoTransactionServer");
    if (tcp_server_ == nullptr)
    {
        LogError() << "make tcp_server failed";
        return -2;
    }

    tcp_server_->setConnectionCallback(
        std::bind(&DemoTransactionServer::OnConnection, this, std::placeholders::_1)
    );

    tcp_server_->setMessageCallback(
        std::bind(&DemoTransactionServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );


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

s32 DemoTransactionServer::Start(u32 thread_num)
{
    tcp_server_->setThreadNum(thread_num);
    tcp_server_->start();
    event_loop_->loop();
    return 0;
}

void DemoTransactionServer::OnConnection(const dwt::TcpConnectionPtr& conn)
{
    LogInfo() << "new connection from " << conn->peerAddress().toIpPort();
    if (! conn->connected())
    {
        LogInfo() << "connection from " << conn->peerAddress().toIpPort() << " closed";
        conn->shutdown();
    }
}

void DemoTransactionServer::OnMessage(const dwt::TcpConnectionPtr& conn, dwt::Buffer* buf, dwt::Timestamp time)
{
    std::string data = buf->retrieveAllAsString();
    LogInfo() << "receive data from " << conn->peerAddress().toIpPort() << " data=" << data;
    conn->send(data);
}