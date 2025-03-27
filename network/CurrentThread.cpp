#include "CurrentThread.h"



namespace dwt{

namespace CurrentThread {

__thread int t_cacheTid = 0;


// 通过系统调用获取当前线程tid值
void cachedTid() {
    if(t_cacheTid == 0) {
        t_cacheTid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
}


} // end CurrentThread dwt


} // end namespace dwt