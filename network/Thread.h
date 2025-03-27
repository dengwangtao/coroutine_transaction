#pragma once

#include "noncopyable.h"
#include <functional>
#include <string>
#include <thread>
#include <memory>
#include <atomic>

namespace dwt {


class Thread : noncopyable {

public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func, const std::string& name = std::string());

    ~Thread();

    void start();

    void join();

    bool started() const { return m_started; }

    pid_t tid() const {return m_tid; }

    const std::string& name() const {return m_name; }

    static int numCreated() { return m_numCreated; }

private:

    void setDefaultName();

    bool m_started;
    bool m_joined;

    std::unique_ptr<std::thread> m_thread;

    pid_t m_tid;
    ThreadFunc m_func;

    std::string m_name;

    static std::atomic<int> m_numCreated;   // 已创建的线程
};


}