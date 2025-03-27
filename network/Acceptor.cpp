#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


namespace dwt {

/**
 * 创建非阻塞的 listenfd
 */
static int createNonBlockingFd() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd == -1) {
        LOG_FATAL("%s:%d %s error: %d",__FILE__, __LINE__, __FUNCTION__, errno);
    }
    return sockfd;
}



Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort)
    : m_loop(loop)
    , m_acceptSocket(createNonBlockingFd())
    , m_acceptChannel(loop, m_acceptSocket.fd())
    , m_listening(false) {

    // 构造
    m_acceptSocket.setReuseAddr(true);
    m_acceptSocket.setReusePort(reusePort);
    m_acceptSocket.bindAddress(listenAddr);
    m_acceptChannel.setReadCallback(
        std::bind(&Acceptor::handleRead, this)
    );
}

Acceptor::~Acceptor() {
    m_acceptChannel.disableAll();
    m_acceptChannel.remove();
}


void Acceptor::listen() {
    m_listening = true;
    m_acceptSocket.listen();
    m_acceptChannel.enableReading();
}


/**
 * 新连接到达
*/
void Acceptor::handleRead() {

    InetAddress peerAddr;
    int connfd = m_acceptSocket.accept(&peerAddr);

    if(connfd >= 0) {
        if(m_newConnectionCallback) {
            // m_newConnectionCallback的功能: 轮询找到 subloop, 并唤醒 subloop, 分发客户端的channel
            m_newConnectionCallback(connfd, peerAddr);
        } else {
            ::close(connfd);
        }
    } else {
        if(errno == EMFILE) {
            LOG_ERROR("%s:%d %s error: socket reached limited",__FILE__, __LINE__, __FUNCTION__);
        } else {
            LOG_ERROR("%s:%d %s error: %d",__FILE__, __LINE__, __FUNCTION__, errno);
        }
    }
}


}