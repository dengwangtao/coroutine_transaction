#include "EventLoopThreadPool.h"

#include "EventLoopThread.h"

namespace dwt {



EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& name)
    :m_baseLoop(baseLoop), m_name(name), m_started(false), m_numThreads(0), m_next(0) {

}
    
EventLoopThreadPool::~EventLoopThreadPool() {
    // nothing
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {

    m_started = true;

    // 创建线程
    for(int i = 0; i < m_numThreads; ++ i) {
        std::string sub_name = m_name + std::to_string(i);

        EventLoopThread* elt = new EventLoopThread(cb, sub_name);
        m_threads.emplace_back(elt);
        m_loops.push_back(elt->startLoop());
    }

    if(m_numThreads == 0 && cb) {
        cb(m_baseLoop);
    }
}


// 轮询的方式给分channel给subloop
EventLoop* EventLoopThreadPool::getNextLoop() {

    EventLoop* loop = m_baseLoop;

    if(!m_loops.empty()) {
        // round-robin
        loop = m_loops[m_next ++];
        // if(static_cast<ssize_t>(m_next) >= m_loops.size()) {
        //     m_next = 0;
        // }
        m_next %= m_loops.size();
    }

    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    if(m_numThreads <= 0) {
        return {m_baseLoop};
    }

    return m_loops;
}


}