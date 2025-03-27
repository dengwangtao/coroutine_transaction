#include "EventLoop.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <memory>

#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

namespace dwt {

// 方式一个线程创建多个EventLoop
__thread EventLoop* t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000; // 默认为10秒

/**
 * 创建一个非阻塞的eventfd
 */
int createEventFd() {
    int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evfd < 0)  {
        LOG_FATAL("create eventfd error");
    }
    return evfd;
}


EventLoop::EventLoop()
    : m_looping(false)
    , m_quit(false)
    , m_callingPendingFunctors(false)
    , m_threadId(CurrentThread::tid())
    , m_poller(Poller::newDefaultPoller(this))
    , m_wakeupFd(createEventFd())
    , m_wakeupChannel(new Channel(this, m_wakeupFd)) {

    //
    LOG_DEBUG("EventLoop created %p in thread %d", this, m_threadId);

    if(t_loopInThisThread != nullptr) {
        LOG_FATAL("another eventloop %p exists in this thread %d", t_loopInThisThread, m_threadId);
    } else {
        t_loopInThisThread = this;
    }

    // 设置wakeupfd的事件类型和回调操作
    m_wakeupChannel->setReadCallback(
        std::bind(&EventLoop::handleRead, this)
    );
    m_wakeupChannel->enableReading();   // 自己的poller监视自己的wakeupChannel, eventfd的EPOLLIN事件
    // enableReading会调用channel的updateChannel, updateChannel会通过Eventloop的updateChannel将m_wakeupChannel注册到m_poller内
}

EventLoop::~EventLoop() {
    m_wakeupChannel->disableAll();
    m_wakeupChannel->remove();
    ::close(m_wakeupFd);
    t_loopInThisThread = nullptr;
}


void EventLoop::loop() {
    m_looping = true;
    m_quit = false;

    LOG_INFO("Eventloop %p start looping", this);

    while(!m_quit) {
        m_activeChannels.clear();
        m_pollReturnTime = m_poller->poll(kPollTimeMs, &m_activeChannels);

        // LOG_INFO("Eventloop %p poll return", this);

        for(Channel* channel : m_activeChannels) {
            // m_currentActiveChannel = channel;
            channel->handleEvent(m_pollReturnTime);
        }

        doPendingFunctors();
    }

    LOG_INFO("Eventloop %p stop looping", this);
    m_looping = false;
}

void EventLoop::quit() {
    m_quit = true;

    if(!isInLoopThread()) {
        wakeup();
    }
}



// 在当前loop中执行cb
void EventLoop::runInLoop(Functor cb) {
    if(isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}
// 将cb放入队列
void EventLoop::queueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pendingFunctors.emplace_back(cb);
    }
    if(!isInLoopThread() || m_callingPendingFunctors) {
        wakeup();
    }
}


// 唤醒当前loop所在线程
void EventLoop::wakeup() {

    // 向wakeupfd写一下数据
    uint64_t one = 1;

    ssize_t n = ::write(m_wakeupFd, static_cast<void*>(&one), sizeof one);

    if(n != sizeof one) {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8", n);
    }

}





void EventLoop::handleRead() {

    uint64_t one = 1;

    ssize_t n = ::read(m_wakeupFd, &one, sizeof one);

    if(n != sizeof one) {
        LOG_ERROR("EventLoop::handleRead() reads %ld bytes instead of 8", n);
    }

}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> funcs;
    m_callingPendingFunctors = true;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        funcs.swap(m_pendingFunctors);
    }

    for(const Functor& f : funcs) {
        f();
    }

    m_callingPendingFunctors = false;
}




void EventLoop::updateChannel(Channel* channel) {
    m_poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    m_poller->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
    return m_poller->hasChannel(channel);
}


}