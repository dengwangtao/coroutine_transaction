#include "EventLoopThread.h"
#include "EventLoop.h"

namespace dwt {


EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name)
    :m_loop(nullptr), m_exiting(false)
    , m_thread(std::bind(&EventLoopThread::threadFunc, this), name)
    , m_threadInitCallback(cb), m_mutex(), m_cond() {
    
    // 构造
}

EventLoopThread::~EventLoopThread() {

    m_exiting = true;
    if(m_loop) {
        m_loop->quit();

        m_thread.join();
    }

}

EventLoop* EventLoopThread::startLoop() {

    m_thread.start();   // 启动新线程, 执行EventLoopThread::threadFunc()函数
    
    EventLoop* loop = nullptr;

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_loop == nullptr) {
            m_cond.wait(lock);
        }
        loop = m_loop;
    }
    return loop;
}


void EventLoopThread::threadFunc() {

    EventLoop loop; // 创建一个独立的loop
    
    if(m_threadInitCallback) {            // 线程初始化完毕的回调
        m_threadInitCallback(&loop);      // 执行ThreadInitCallback函数
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_loop = &loop;

        m_cond.notify_one();    // 通知, m_loop已经和EventLoopThread绑定了
    }

    loop.loop();    // 开启事件循环 m_loop->loop();
    

    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_loop = nullptr;
}


}