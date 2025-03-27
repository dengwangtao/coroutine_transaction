#include "Socket.h"
#include "InetAddress.h"
#include "Logger.h"

#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>


namespace dwt {

Socket::~Socket() {
    ::close(m_socketFd);
}


void Socket::bindAddress(const InetAddress& localaddr) {
    sockaddr_in* addr = const_cast<sockaddr_in*>(localaddr.getSockAddr());
    int n = ::bind(m_socketFd, reinterpret_cast<struct sockaddr*>(addr), sizeof(*addr));
    if(n != 0) {
        LOG_FATAL("Socket::bindAddress bind sockfd Error: %d", errno);
    }
}

void Socket::listen() {
    int n = ::listen(m_socketFd, 10);
    if(n != 0) {
        LOG_FATAL("Socket::listen Error: %d", errno);
    }
}

/**
 * peerAddr 输出参数
*/
int Socket::accept(InetAddress* peerAddr) {

    sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    socklen_t len = static_cast<socklen_t>(sizeof addr);

    // int connfd = ::accept(m_socketFd, (sockaddr*)&addr, &len);
    // 设置返回的用于通信的文件描述符为非阻塞
    int connfd = ::accept4(m_socketFd, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if(connfd >= 0) {
        peerAddr->setSockAddr(addr);
    }

    return connfd;
}

void Socket::shutdownWrite() {
    if(-1 == ::shutdown(m_socketFd, SHUT_WR)) {
        LOG_ERROR("Socket::shutdownWrite() Error");
    }
}

void Socket::setTcpNoDelay(bool on) {

    int optval = on ? 1 : 0;

    if(-1 == ::setsockopt(m_socketFd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval))) {
        LOG_ERROR("Socket::setTcpNoDelay(%d) Error", optval);
    }

}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;

    if(-1 == ::setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval))) {
        LOG_ERROR("Socket::setReuseAddr(%d) Error", optval);
    }
}

void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;

    if(-1 == ::setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval))) {
        LOG_ERROR("Socket::setReusePort(%d) Error", optval);
    }
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;

    if(-1 == ::setsockopt(m_socketFd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval))) {
        LOG_ERROR("Socket::setKeepAlive(%d) Error", optval);
    }
}


}