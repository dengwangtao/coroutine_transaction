#pragma once

#include "noncopyable.h"

#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "Thread.h"

namespace dwt {

// 前置声明
class EventLoop;



class EventLoopThread : noncopyable {

public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const std::string& name = std::string());

    ~EventLoopThread();

    EventLoop* startLoop();

private:

    void threadFunc();


    EventLoop* m_loop;

    bool m_exiting;

    dwt::Thread m_thread;

    ThreadInitCallback m_threadInitCallback;

    std::mutex m_mutex;
    std::condition_variable m_cond;
};



}