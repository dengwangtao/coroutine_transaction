#include "Channel.h"

#include <sys/epoll.h>

#include "EventLoop.h"
#include "Logger.h"

namespace dwt {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI; // 文件可读 | 文件有紧急数据可读
const int Channel::kWriteEvent = EPOLLOUT;


Channel::Channel(EventLoop* loop, int fd)
    : m_fd(fd), m_loop(loop), m_events(0), m_revents(0), m_index(-1), m_tied(false) {
    
    // 一个EventLoop管理多个Channel
}

Channel::~Channel() {

}


void Channel::handleEvent(Timestamp receiveTime) {
    if(m_tied) {
        std::shared_ptr<void> guard = m_tie.lock();
        if(guard) {
            handelEventWithGuard(receiveTime);
        }
    } else {
        handelEventWithGuard(receiveTime);
    }

}

void Channel::setReadCallback(ReadEventCallback cb) {
    m_readCallback = std::move(cb);
}
void Channel::setWriteCallback(EventCallback cb) {
    m_writeCallback = std::move(cb);
}
void Channel::setCloseCallback(EventCallback cb) {
    m_closeCallback = std::move(cb);
}
void Channel::setErrorCallback(EventCallback cb) {
    m_errorCallback = std::move(cb);
}


void Channel::tie(const std::shared_ptr<void>& obj) {
    m_tie = obj;    // weak_ptr 观察 shared_ptr
    m_tied = true;
}

int Channel::fd() const {
    return m_fd;
}

int Channel::events() const {
    return m_events;
}

int Channel::index() const {
    return m_index;
}

void Channel::set_index(int index) {
    m_index = index;
}


void Channel::set_revents(int revents) {
    m_revents = revents;
}


void Channel::enableReading() {
    m_events |= kReadEvent;
    update();
}
void Channel::disableReading() {
    m_events &= ~kReadEvent;
    update();
}
void Channel::enableWriting() {
    m_events |= kWriteEvent;
    update();
}
void Channel::disableWriting() {
    m_events &= ~kWriteEvent;
    update();
}
void Channel::disableAll() {
    m_events = kNoneEvent;
    update();
}


bool Channel::isNoneEvent() const {
    return m_events == kNoneEvent;
}
bool Channel::isWriting() const {
    return m_events & kWriteEvent;
}
bool Channel::isReading() const {
    return m_events & kReadEvent;
}


EventLoop* Channel::ownerLoop() {
    return m_loop;
}

void Channel::remove() {
    // 通过EventLoop让Poller删除Channel
    // m_loop->removeChannel(this);
    m_loop->removeChannel(this);
}


void Channel::update() {
    // 通过EventLoop让Poller更新Channel状态
    // m_loop->updateChannel(this);
    m_loop->updateChannel(this);
}

void Channel::handelEventWithGuard(Timestamp reveiveTime) {

    LOG_INFO("channel handleEvent events: %d", m_revents);

    //EPOLLHUP 表示读写都关闭
    if((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN)) {
        if(m_closeCallback) {
            m_closeCallback();
        }
    }

    if(m_revents & EPOLLERR) {
        if(m_errorCallback) {
            m_errorCallback();
        }
    }

    if(m_revents & (EPOLLIN | EPOLLPRI)) {
        if(m_readCallback) {
            m_readCallback(reveiveTime);
        }
    }

    if(m_revents & EPOLLOUT) {
        if(m_writeCallback) {
            m_writeCallback();
        }
    }

}



}