#include "coroutine_scheduler.h"
#include <unistd.h>
#include "coroutine_mem.h"
#include "gen_guid.h"

CoroutineScheduler::CoroutineScheduler()
{
    // alloc_id_ = 0;
    curr_routine_id_ = 0;
    page_size_ = 0;
}

CoroutineScheduler::~CoroutineScheduler()
{

}

s32 CoroutineScheduler::Init()
{
    // 获取系统页大小，用于后面做堆栈溢出的保护
    long sz = sysconf(_SC_PAGESIZE);
    page_size_ = static_cast<s32>(sz);
    LogInfo() << "page size is" << page_size_;

    // 创建主协程
    s32 ret = main_coroutine_.CreateMainRountine();
    if(ret != 0)
    {
        LogError() << "create main coroutine failed, ret " << ret;
        return -1;
    }

    return 0;
}

s32 CoroutineScheduler::CreateWorkRoutine(s32 stack_size, COROUTINE_FUNC func, void* param, u64& id)
{
    u64 new_id = AllocCoroutineID();
    CoroutineImpl* co = CoroutineMem::CreateCoroutine();
    if(nullptr == co)
    {
        LogError() << "new coroutine failed.";
        return -1;
    }

    s32 ret = co->CreateWorkRoutine(main_coroutine_, new_id, stack_size, page_size_, func, param);
    if(ret != 0)
    {
        LogError() << "CreateWorkRoutine failed, ret " << ret;
        co->Destroy();
        CoroutineMem::DestroyCoroutine(co);
        return -2;
    }

    auto result = coroutines_map_.insert(std::make_pair(new_id, co));
    if(!result.second)
    {
        co->Destroy();
        CoroutineMem::DestroyCoroutine(co);
        return -3;
    }

    id = new_id;
    return 0;
}

s32 CoroutineScheduler::DestroyWorkRoutine(u64 id)
{
    if (id == 0)
    {
        // error_tlog("invalid id.");
        return -1;
    }

    auto iter = coroutines_map_.find(id);
    if (iter == coroutines_map_.end())
    {
        // error_tlog("coroutine <%lu> not exist.", id);
        return -2;
    }

    if (iter->second != NULL)
    {
        iter->second->Destroy();
        CoroutineMem::DestroyCoroutine(iter->second);
    }

    coroutines_map_.erase(iter);
    return 0;
}


s32 CoroutineScheduler::SwapToMain()
{
    if(curr_routine_id_ == 0)
    {
        // error_tlog("already in main coroutine.");
        return -1;
    }

    auto iter = coroutines_map_.find(curr_routine_id_);
    if(iter == coroutines_map_.end())
    {
        // error_tlog("curr routine not exist. may exist some error.");
        return -2;
    }

    // 必须先置， 否则swap之后下面的代码不被执行，就没法再设置id了
    curr_routine_id_ = 0;
    s32 ret = 0;
    ret = main_coroutine_.Resume(*iter->second);
    if(ret != 0)
    {
        // error_tlog("resume failed, ret<%d>.", ret);
        curr_routine_id_ = iter->second->id();
        return ret;
    }

    return 0;
}

s32 CoroutineScheduler::SwapToWorkRoutine(u64 new_routine_id)
{
    if(curr_routine_id_ != 0)
    {
        // error_tlog("already in work coroutine.");
        return -1;
    }

    auto iter = coroutines_map_.find(new_routine_id);
    if(iter == coroutines_map_.end())
    {
        // error_tlog("new routine <%lu> not exist. may exist some error.", new_routine_id);
        return -2;
    }

    // 必须先置， 否则swap之后下面的代码不被执行，就没法再设置id了
    curr_routine_id_ = iter->second->id();
    s32 ret = 0;
    ret = iter->second->Resume(main_coroutine_);
    if(ret != 0)
    {
        // error_tlog("resume failed, ret<%d>.", ret);
        curr_routine_id_ = 0;
        return ret;
    }
    return 0;
}

CoroutineImpl* CoroutineScheduler::GetCoroutineById(u64 id)
{
	if (0 == id)
	{
		// error_tlog("Invalid coroutine id");
		return NULL;
	}

	auto iter = coroutines_map_.find(id);
	if (iter == coroutines_map_.end())
	{
		// error_tlog("routine <%lu> not exist. may exist some error.", id);
		return NULL;
	}
	return iter->second;
}

s32 CoroutineScheduler::OnWorkRoutineExit()
{
    if (curr_routine_id_ != 0)
    {
        auto *co = GetCoroutineById(curr_routine_id_);
        if (co)
        {
            co->set_state(E_COROUTINE_STATE_DEAD);
        }
    }
    curr_routine_id_ = 0;
    main_coroutine_.set_state(E_COROUTINE_STATE_RUN);

    return 0;
}

u64 CoroutineScheduler::AllocCoroutineID()
{
    // ++alloc_id_;
    // if(alloc_id_ == 0)
    // {
    //     alloc_id_ = 1;
    // }

    // return alloc_id_;

    return GenGUID();
}

s32 CoroutineScheduler::Resume()
{
    curr_routine_id_ = 0;

    s32 ret = main_coroutine_.RestartMainRoutine();
    if(ret != 0)
    {
        // error_tlog("restart main routine failed, ret<%d>", ret);
    }

    return ret;
}

s32 CoroutineScheduler::RestartCoroutine(u64 id, COROUTINE_FUNC func, void* param)
{
    auto it = coroutines_map_.find(id);
    if(it == coroutines_map_.end() || it->second == NULL)
    {
        // error_tlog("coroutine <%lu> doesn't exist.", id);
        return -1;
    }

    s32 ret = it->second->RestartWorkRoutine(main_coroutine_, func, param);
    if(ret != 0)
    {
        // error_tlog("couroutine<%lu> restart failed", id);
    }
    return ret;
}

