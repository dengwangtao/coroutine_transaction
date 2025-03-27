#include "trans_server.h"
#include "trans.h"
#include "transaction_server.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "peer_mgr.h"
#include "gm.h"

s32 DemoTransactionServer::Init()
{
    LogDebug() << "DemoTransactionServer::Init";
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

    if (! g_trans_server_ptr)
    {
        LogFatal() << "Init DemoTransactionServer failed, g_trans_server_ptr is nullptr";
        return -3;
    }

    LogDebug() << "DemoTransactionServer::Init done";
    return 0;
}

Transaction* DemoTransactionServer::GetTranByType(s32 type) const
{
    switch (type)
    {
    case E_TRANSACTION_TYPE_DEMO:
        return &DemoTransaction::Instance();
    case E_TRANSACTION_TYPE_DEMO_2:
        return &DemoTransaction2::Instance();
        
    default:
        LogError() << "unknown tran type=" << type;
        break;
    }

    return nullptr;
}

s32 DemoTransactionServer::Start()
{
    tcp_server_->setThreadNum(1);
    tcp_server_->start();
    event_loop_->loop();
    return 0;
}

void DemoTransactionServer::OnConnection(const dwt::TcpConnectionPtr& conn)
{
    auto peer = conn->peerAddress().toIpPort();
    
    if (! conn->connected())
    {
        g_peer_mgr.remove_peer(peer);
        LogInfo() << "connection from " << peer << " closed";
        conn->shutdown();
    }
    else
    {
        auto peer_id = g_peer_mgr.add_peer(peer);
        LogInfo() << "new connection from " << peer << " peer_id=" << peer_id;
    }
}

void DemoTransactionServer::OnMessage(const dwt::TcpConnectionPtr& conn, dwt::Buffer* buf, dwt::Timestamp time)
{
    auto peer = conn->peerAddress().toIpPort();
    auto peer_id = g_peer_mgr.get_peer_id(peer);
    if (peer_id == 0)
    {
        LogError() << "peer_id not found for " << peer;
        peer_id = g_peer_mgr.add_peer(peer);
        LogInfo() << "create new peer_id=" << peer_id << " for " << peer;
    }

    std::string data = buf->retrieveAllAsString();
    LogInfo()   << "[thread=" << std::this_thread::get_id() << "] " 
                << "receive data from " << conn->peerAddress().toIpPort() << " data=" << data;

    std::string ret_str;
    s32 ret = g_gm_mgr.CallGM(peer_id, data, &ret_str);
    
    conn->send(ret_str + "; ret=" + std::to_string(ret) + "\n");
}