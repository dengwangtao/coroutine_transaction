#pragma once

#include "noncopyable.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "Callbacks.h"
#include "TcpConnection.h"

// 统一包含
#include "Logger.h"
#include "Poller.h"
#include "Thread.h"


#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <unordered_map>


namespace dwt {


/**
 * TcpServer
 */
class TcpServer : noncopyable {

public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum class Option {
        kNoReusePort,
        kReusePort
    };

    TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name, Option option = Option::kNoReusePort);
    ~TcpServer();

    const std::string& ipPort() const { return m_ipPort; }
    const std::string& name() const { return m_name; }
    EventLoop* getLoop() const { return m_loop; }

    void setThreadInitCallback(const ThreadInitCallback& cb) { m_threadInitCallback = cb; }

    void setThreadNum(int numThreads);

    void start();

    void setConnectionCallback(const ConnectionCallback& cb) { m_connectionCallback = cb; }

    void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; }

    // void setCloseCallback(const CloseCallback& cb) { m_closeCallback = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCallback = cb; }

private:

    void newConnection(int sockfd, const InetAddress& peerAddr);

    void removeConnection(const TcpConnectionPtr& conn);

    void removeConnectionInLoop(const TcpConnectionPtr& conn);


    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;


    EventLoop* m_loop;          // baseLoop, 用户传入
    const std::string m_ipPort;
    const std::string m_name;

    std::unique_ptr<Acceptor> m_acceptor;   // 运行在mainLoop, 监听新连接事件
    std::shared_ptr<EventLoopThreadPool> m_threadPool;  // one loop per thread

    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    // CloseCallback m_closeCallback;
    WriteCompleteCallback m_writeCompleteCallback;

    ThreadInitCallback m_threadInitCallback;

    std::atomic<int> m_started;

    ConnectionMap m_connections;    //保存所有连接

    int m_nextConnId;

};


}