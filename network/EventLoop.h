#pragma once


#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

#include "CurrentThread.h"
#include "Timestamp.h"

#include "noncopyable.h"

namespace dwt {

// 前置声明
class Channel;
class Poller;


class EventLoop : noncopyable {

// 包含两个模块: Channel, Poller

public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();    // 开启事件循环

    void quit();

    Timestamp pollReturnTime() const {
        return m_pollReturnTime;
    }

    // 在当前loop中执行cb
    void runInLoop(Functor cb);
    // 将cb放入队列
    void queueInLoop(Functor cb);

    // 唤醒当前loop所在线程
    void wakeup();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    // 判断当前eventloop对象是否在自己线程内
    bool isInLoopThread() const {
        return m_threadId == CurrentThread::tid();
    }

private:

    void handleRead();

    void doPendingFunctors();

private:

    using ChannelList = std::vector<Channel*>;

    std::atomic<bool> m_looping;    // 原子操作, 通过CAS实现
    std::atomic<bool> m_quit;
    
    const pid_t m_threadId;
    Timestamp m_pollReturnTime;
    std::unique_ptr<Poller> m_poller;

    int m_wakeupFd;                 // eventfd()
    std::unique_ptr<Channel> m_wakeupChannel;

    ChannelList m_activeChannels;
    // Channel* m_currentActiveChannel;

    std::atomic<bool> m_callingPendingFunctors; //标识当前loop是否有需要执行的回调操作
    std::vector<Functor> m_pendingFunctors; // 所有的回调操作
    std::mutex m_mutex;                     // 保证m_pendingFunctors线程安全
};


}