#pragma once

#include "noncopyable.h"

#include <vector>
#include <memory>
#include <functional>
#include <string>

namespace dwt {


// 前置声明
class EventLoop;
class EventLoopThread;


class EventLoopThreadPool : noncopyable {

public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
    
    ~EventLoopThreadPool();

    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    void setThreadNum(int numThreads) {m_numThreads = numThreads; }

    EventLoop* getNextLoop();   // 轮询的方式给分channel给subloop

    std::vector<EventLoop*> getAllLoops();

    bool started() const { return m_started; }

    const std::string& name() const { return m_name; }

private:


    EventLoop* m_baseLoop;          // 返回给用户

    std::string m_name;
    bool m_started;
    int m_numThreads;
    int m_next;

    std::vector<std::unique_ptr<EventLoopThread>> m_threads;
    std::vector<EventLoop*> m_loops;
};





}