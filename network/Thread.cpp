#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>

namespace dwt {

std::atomic<int> Thread::m_numCreated{0};

Thread::Thread(ThreadFunc func, const std::string& name)
    :m_started(false), m_joined(false), m_tid(0), m_func(std::move(func)), m_name(name) {

    // 构造函数
    setDefaultName();
}

Thread::~Thread() {
    if(m_started && !m_joined) {
        m_thread->detach();
    }

}

void Thread::start() {
    m_started = true;

    sem_t sem;
    sem_init(&sem, false, 0);

    m_thread = std::make_unique<std::thread>([&]() {
        // 获取线程tid
        m_tid = dwt::CurrentThread::tid();
        sem_post(&sem);

        m_func();
    });

    sem_wait(&sem);
    // 保证线程获取到线程id后, 该函数才能返回
}

void Thread::join() {
    m_joined = true;

    m_thread->join();
}


void Thread::setDefaultName() {
    int num = ++ m_numCreated;
    if(m_name.empty()) {
        char buf[32] = {0};
        snprintf(buf, 32, "Thread-%d", num);
        m_name = buf;
    }
}


}