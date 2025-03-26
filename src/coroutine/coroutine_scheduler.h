#pragma once
#ifndef COROUTINE_SCHEDULER_H
#define COROUTINE_SCHEDULER_H

#include "coroutine_def.h"
#include "coroutine.h"
#include <unordered_map>
#include "singleton.h"

// 协程管理器
class CoroutineScheduler
{
public:
    CoroutineScheduler();
    ~CoroutineScheduler();

public:
    s32 Init();
    // 创建工作协程
    s32 CreateWorkRoutine(s32 stack_size,  COROUTINE_FUNC func, void* param, u64& id);
    // 销毁工作协程
    s32 DestroyWorkRoutine(u64 id);

    // 从当前协程切换到主协程,不允许协程自己切，只允许普通协程和主协程进行切换
    s32 SwapToMain();
    // 只允许从协程管理器调用切换
    s32 SwapToWorkRoutine(u64 new_routine_id);

    CoroutineImpl* GetCoroutineById(u64 id);

    CoroutineImpl& main_routine() { return main_coroutine_; }
    s32 page_size() { return page_size_; }
    // 处理工作协程退出的情况
    s32 OnWorkRoutineExit();

    s32 Resume();

    s32 RestartCoroutine(u64 id, COROUTINE_FUNC func, void* param);

    bool IsInCoroutine() const
    {
        return curr_routine_id_ != 0;
    }

    u64 curr_routine_id()const { return curr_routine_id_; }

private:
    // 分配一个协程ID, 全局唯一
    u64 AllocCoroutineID();

private:
    std::unordered_map<u64, CoroutineImpl*> coroutines_map_; // 工作协程集合

    CoroutineImpl main_coroutine_; // 主协程
    u64 curr_routine_id_; // 当前coroutine id. 0为主协程
    s32 page_size_; // 当前机器的页大小，一般默认为4KB
    // u64 alloc_id_;  // 分配协程ID
};

#endif // COROUTINE_SCHEDULER_H
