#pragma once

#include "coroutine_def.h"
#include "boost_context.h"

// 单个协程实现,
// 只能创建销毁协程，不能切换，这样可以保证所有的协程切换都是可控的
// 只能在主协程和非主协程直接切换
class BoostCoroutine
{
public:
    BoostCoroutine() = default;

    ~BoostCoroutine();

    friend class CoroutineScheduler;

    BoostCoroutine(const BoostCoroutine &) = delete;
    BoostCoroutine& operator=(const BoostCoroutine &) = delete;

private:
    // 创建协程
    s32 CreateWorkRoutine(BoostCoroutine& main, u64 id, s32 stack_size,
                          s32 page_size, COROUTINE_FUNC func, void* param);

    // 创建主协程
    s32 CreateMainRountine();
    // 销毁协程
    void Destroy();
    // 恢复当前协程
    s32 Resume(BoostCoroutine& old_routine);

    s32 Prepare(BoostCoroutine& main, COROUTINE_FUNC func, void* param);
    // 返回主协程,
    s32 BackToMain(BoostCoroutine& main);

public:
    s32 RestartMainRoutine();
    s32 RestartWorkRoutine(BoostCoroutine& main, COROUTINE_FUNC func, void* param);

    u64 id() const { return id_; }

    s32 state() const { return state_; }
    void set_state(s32 state) { state_ = state; }

    BoostContext &context() { return context_; }
    bool is_main() const { return is_main_; }

private:
    u64 id_ = 0;          // 协程的唯一ID
    char* stack_ = nullptr;     // 堆栈指针
    void* param_ = nullptr;  // 保存参数
    BoostContext context_ = nullptr;
    s32 state_ = E_COROUTINE_STATE_UNINIT;
    s32 stack_size_ = 0;
    s32 page_size_ = 0;
    u32 valgrind_id_ = 0;
    bool is_main_ = false;    // 是否主协程
};

