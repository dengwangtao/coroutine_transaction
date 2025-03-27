#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

namespace dwt {

class EventLoop;    // 前置声明, 方便下面使用

/**
 * 封装了sockfd和其感兴趣的event: EPOLLIN EPOLLOUT等事件
 * 还绑定了poller返回的具体事件
*/
class Channel : noncopyable {
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;
public:
    Channel(EventLoop* loop, int fd);
    ~Channel();

    // fd得到poller通知后, 处理事件
    void handleEvent(Timestamp receiveTime);
    void setReadCallback(ReadEventCallback cb);
    void setWriteCallback(EventCallback cb);
    void setCloseCallback(EventCallback cb);
    void setErrorCallback(EventCallback cb);

    // 防止当TcpConnection被remove, channel还在执行TcpConnection的回调操作
    void tie(const std::shared_ptr<void>& obj);

    int fd() const;
    int events() const;
    int index() const;
    void set_index(int index);


    void set_revents(int revents);

    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();
    void disableAll();

    bool isNoneEvent() const;
    bool isWriting() const;
    bool isReading() const;

    EventLoop* ownerLoop();
    void remove();

private:

    void update();
    void handelEventWithGuard(Timestamp reveiveTime);

private:

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop*      m_loop;     // 事件循环
    const int       m_fd;       // fd, poller监听的对象
    int             m_events;   // 感兴趣的事件
    int             m_revents;  // 具体发生的事件
    int             m_index;    // 

    std::weak_ptr<void> m_tie;  // 用来监视TcpConnection
    bool                m_tied;

    ReadEventCallback   m_readCallback;
    EventCallback       m_writeCallback;
    EventCallback       m_closeCallback;
    EventCallback       m_errorCallback;
};


}