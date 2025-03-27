#pragma once

#include "noncopyable.h"

namespace dwt {

class InetAddress;


/**
 * 封装socket fd
*/
class Socket : noncopyable {

public:
    explicit Socket(int sockfd) : m_socketFd(sockfd) {}
    ~Socket();

    int fd() const { return m_socketFd; }

    void bindAddress(const InetAddress& localaddr);

    void listen();

    int accept(InetAddress* peerAddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);

    void setReuseAddr(bool on);

    void setReusePort(bool on);

    void setKeepAlive(bool on);

private:
    const int m_socketFd;

};


}