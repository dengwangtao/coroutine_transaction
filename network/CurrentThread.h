#pragma once

#include <unistd.h>
#include <sys/syscall.h>
#include "noncopyable.h"

namespace dwt{

namespace CurrentThread {


// 线程内全局变量
extern __thread int t_cacheTid; // (变量声明)


// 通过系统调用获取当前线程tid值
void cachedTid();



inline int tid() {
    if(__builtin_expect(t_cacheTid == 0, 0)) { // 分支预测, cachedTid()执行的可能性小
        cachedTid();
    }
    return t_cacheTid;
}



} // end CurrentThread dwt


} // end namespace dwt