#include "TcpServer.h"

#include "Logger.h"
#include "TcpConnection.h"

#include <string.h>

namespace dwt {

static EventLoop* checkLoopNotNull(EventLoop* loop) {
    if(loop == nullptr) {
        LOG_FATAL("%s %s %d mainLoop is nullptr", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(
    EventLoop* loop, const InetAddress& listenAddr,
    const std::string& name, Option option)
        : m_loop(checkLoopNotNull(loop))
        , m_ipPort(listenAddr.toIpPort())
        , m_name(name)
        , m_acceptor(std::make_unique<Acceptor>(loop, listenAddr, option == Option::kReusePort))
        , m_threadPool(std::make_shared<EventLoopThreadPool>(loop, name))
        , m_connectionCallback()
        , m_messageCallback()
        , m_started(0)
        , m_nextConnId(1) {
    
    // 构造

    m_acceptor->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2)
    );


}
TcpServer::~TcpServer() {
    for(auto& item : m_connections) {
        TcpConnectionPtr conn(item.second);

        item.second.reset(); // 让item.second不再指向资源, 只让conn指向资源

        conn->getLoop()->queueInLoop(
            std::bind(&TcpConnection::connectDestoryed, conn)
        );
    }
}


void TcpServer::setThreadNum(int numThreads) {
    m_threadPool->setThreadNum(numThreads);
}

void TcpServer::start() {
    if(m_started ++ == 0) {
        m_threadPool->start(m_threadInitCallback);

        m_loop->runInLoop(
            std::bind(&Acceptor::listen, m_acceptor.get())
        );
    }
}



void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {

    EventLoop* ioLoop = m_threadPool->getNextLoop();

    std::string connName = m_ipPort + "#" + std::to_string(m_nextConnId);
    ++ m_nextConnId;

    LOG_DEBUG("TcpServer::newConnection: %s", connName.c_str());

    sockaddr_in local;
    ::memset(&local, 0, sizeof local);
    socklen_t socklen = static_cast<socklen_t>(sizeof local);
    if(::getsockname(sockfd, (sockaddr*)&local, &socklen) < 0) {
        LOG_ERROR("TcpServer::newConnection getsockname error");
    }

    InetAddress localAddr(local);

    TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop, connName, sockfd, localAddr, peerAddr);

    m_connections[connName] = conn;

    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)
    );
    
    ioLoop->runInLoop(
        std::bind(&TcpConnection::connectEstablished, conn)
    );
    // ioLoop->wakeup();

}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {

    m_loop->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );

}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {

    std::string connName = conn->name();

    LOG_INFO("TcpServer::removeConnectionInLoop [%s] connection[%s]", m_name.c_str(), connName.c_str());

    m_connections.erase(connName);

    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestoryed, conn)
    );
}


}