#pragma once

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

namespace dwt {


class EventLoop;
class InetAddress;


/**
 * Acceptor: 传入baseLoop
 * 构造时 创建一个监听的文件描述符listenfd, 和对应的Channel
 */
class Acceptor : noncopyable {

public:
    using NewConnectionCallback = std::function<void(int fd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort);

    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        m_newConnectionCallback = cb;
    }

    bool listening() const { return m_listening; }

    void listen();

private:

    void handleRead();

    EventLoop* m_loop;
    Socket m_acceptSocket;
    Channel m_acceptChannel;

    bool m_listening;

    NewConnectionCallback m_newConnectionCallback;
};


}