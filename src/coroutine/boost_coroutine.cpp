#include "boost_coroutine.h"

#include <sys/mman.h>
#include <stdlib.h>

#include "boost_context.h"
#include "common_def.h"
#include "valgrind.h"

BoostCoroutine::~BoostCoroutine()
{
}

s32 BoostCoroutine::CreateWorkRoutine(BoostCoroutine& main, u64 id, s32 stack_size, s32 page_size, COROUTINE_FUNC func, void* param)
{
    if (state_ != E_COROUTINE_STATE_UNINIT)
    {
        LogError() << "already inited.";
        return -1;
    }

    if (stack_size <= 0 || page_size <= 0 || id == 0 || nullptr == func)
    {
        LogError() << "invalid args";
        return -1;
    }

    // page_size 必须是2的幂
    if ((page_size & (page_size - 1)) != 0) {
        LogError() << "page size " << page_size << " is not a power of two";
        return -1;
    }

    is_main_ = false;
    stack_size_ = stack_size;
    page_size_ = page_size;

    id_ = id;

    s32 ret = Prepare(main, func, param);
    if (ret != 0)
    {
        LogError() << "Prepare failed<"<< ret << ">, coroutine<" << id << ">";
    }

    return ret;
}

s32 BoostCoroutine::CreateMainRountine()
{
    if (state_ != E_COROUTINE_STATE_UNINIT)
    {
        LogError() << "already inited.";
        return -1;
    }

    is_main_ = true;
    state_ = E_COROUTINE_STATE_RUN;

    return 0;
}

void BoostCoroutine::Destroy()
{
    if (stack_ == nullptr)
    {
        return;
    }

    int ret = mprotect(stack_, page_size_, PROT_READ | PROT_WRITE);
    if (ret != 0)
    {
        LogError() << "mprotect error, errno<"<< ret << ">.";
    }
    ret = mprotect(stack_ + stack_size_ + page_size_, page_size_, PROT_READ | PROT_WRITE);
    if (ret != 0)
    {
        LogError() << "mprotect error, errno<"<<errno<<">.";
    }

    VALGRIND_STACK_DEREGISTER(valgrind_id_);
    free(stack_);
    stack_ = nullptr;
    stack_size_ = 0;
}

s32 BoostCoroutine::Resume(BoostCoroutine& old_routine)
{
    if (state_ != E_COROUTINE_STATE_SUSPEND && state_ != E_COROUTINE_STATE_READY)
    {
        LogError() << "invalid state<"<<state_<<">.";
        return -1;
    }

    if (old_routine.is_main() == is_main())
    {
        LogError() << "coroutine type is same.cannot swap.";
        return -1;
    }

    set_state(E_COROUTINE_STATE_RUN);
    old_routine.set_state(E_COROUTINE_STATE_SUSPEND);

    (void) JumpContext(&old_routine.context(), context_, param_, false);

    return 0;
}

s32 BoostCoroutine::Prepare(BoostCoroutine& main, COROUTINE_FUNC func, void* param)
{
    param_ = param;
    size_t real_size = 2 * page_size_ + stack_size_;
    stack_ = nullptr;
    int ret = posix_memalign(reinterpret_cast<void**>(&stack_), page_size_, real_size);
    if (ret != 0)
    {
        LogError() << "new stack failed, stack size="<<real_size<<", ret="<<ret;
        return -2;
    }

    valgrind_id_ = VALGRIND_STACK_REGISTER(stack_, stack_ + real_size);

    ret = mprotect(stack_, page_size_, PROT_NONE);
    if (ret != 0)
    {
        LogError() << "mprotect preceding error, errno=errno" << errno;
    }

    ret = mprotect(stack_ + stack_size_ + page_size_, page_size_, PROT_NONE);
    if (ret != 0)
    {
        LogError() << "mprotect following error, errno="<<errno;
    }

    context_ =  MakeContext(stack_ + page_size_+ stack_size_, stack_size_, func);
    state_ = E_COROUTINE_STATE_READY;

    return 0;
}

s32 BoostCoroutine::BackToMain(BoostCoroutine &main)
{
    JumpContext(&context_, main.context(), param_, false);
    return 0;
}

s32 BoostCoroutine::RestartMainRoutine()
{
    if (!is_main_)
    {
        LogError() << "not main coroutine";
        return -1;
    }

    context_ = nullptr;
    state_ = E_COROUTINE_STATE_RUN;

    return 0;
}

s32 BoostCoroutine::RestartWorkRoutine(BoostCoroutine &main, COROUTINE_FUNC func, void* param)
{
    if (is_main_)
    {
        LogError() << "logic error, not work routine";
        return -1;
    }

    stack_ = nullptr;

    // The crash occurs while the crash is running. We don't restart the
    // coroutine in case it crashes again.
    if (state_ == E_COROUTINE_STATE_RUN)
    {
        LogError() << "possibly crashed in coroutine<"<<id_<<">, wont restart";
        return -1;
    }

    if (state_ == E_COROUTINE_STATE_UNINIT ||
        state_ == E_COROUTINE_STATE_DEAD)
    {
        LogWarn() << "coroutine<"<<id_<<"> invalid state<"<<state_<<">";
        return -2;
    }

    s32 ret = Prepare(main, func, param);
    if (ret != 0)
    {
        LogError() << "Prepare failed<"<<ret<<">, coroutine<"<<id_<<">";
    }

    return ret;
}
