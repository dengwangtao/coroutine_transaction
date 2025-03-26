#include "svr_timer.h"
#include "datetime.h"
#include "gen_guid.h"
#include "common_def.h"
#include "transaction/transaction_server.h"

static u16 wheel_size[] = {
    1000,
    3600,
    24
};

#ifdef TEST_REWINDING    //测试时间回绕
# define REWINDING_TIME   (1000*3600*24)
# define CAL_EXPIRE(T,MS) (((T) + (MS)) % REWINDING_TIME)
# define INC_LASTTICK(T)  ((T) = ((T) + 1) % REWINDING_TIME)
#else
# define REWINDING_TIME   0xFFFFFFFFFFFFFFFF
# define CAL_EXPIRE(T,MS) ((T) + (MS))
# define INC_LASTTICK(T)  (++(T))
#endif

static inline u32 cal_remain(u32 now, u32 expire)
{
    if(now > expire)
    {
        //出现时间回绕
        return (REWINDING_TIME - now) + expire;
    } else return expire - now;
}


namespace TimerMem {
    Timer* CreateTimer()
    {
        return new Timer();
    }

    void DestroyTimer(Timer *timer)
    {
        delete timer;
    }
}





TIMEOUTCBFUNC Timer::timer_func_array[];

s32 Timer::OnTimeout(u64 curms)
{
    TIMEOUTCBFUNC func = timer_func_array[timer_func_id_];
    if(func == NULL)
    {
        LogError() << "unregister timer func id=" << timer_func_id_;
        return -1;
    }
    // 回调中增加 timer_id 参数
    func(timer_id_, data_, data_len_);
    return 0;
}

Timer::~Timer()
{
}

s32 Timer::RegisterTimerFunc(u32 timer_func_id, TIMEOUTCBFUNC func)
{
    if(timer_func_id <= E_BASE_TIMER_FUNC_ID_INVALID || timer_func_id >= MAX_TIMER_FUNC_ID_NUM)
    {
        LogError() << "invalid timer func id=" << timer_func_id;
        return -1;
    }
    if(timer_func_array[timer_func_id] != NULL)
    {
        LogError() << "timer func id repeated. id=" << timer_func_id;
        return -2;
    }
    timer_func_array[timer_func_id] = func;
    return 0;
}

void TimerMgr::Proc()
{
    if(NULL == tick_)
    {
        return;//没有注册过定时器
    }

    u64 now = DateTime::GetNowSteadyMSec();
    if(now < lasttick_)
    {
        // 发生时间倒回的话，之前的定时器无法在指定时间超时。暂时先保证下面loop不
        // 会出现卡住CPU的情况，之前的定时器如果有需求正常超时，后面可能需要改成
        // 使用可靠的时钟如系统启动以来的时间或记录下这次offset，在后面处理时候
        // 考虑这个偏移。
        lasttick_ = now;
        return;
    }

    while(lasttick_ != now)
    {
        INC_LASTTICK(lasttick_);
        Fire(&wheels_[WHEEL_TYPE_SEC], lasttick_);
    }
}


void TimerMgr::Reg(Timer *timer, u64 tick)
{
    u32 slot, wsize;
    TimeWheel   *wheel = NULL;
    s32 type = WHEEL_TYPE_SEC;
    u32 remain = cal_remain(tick, timer->expire_);
    do
    {
        wheel = &wheels_[type];
        wsize = wheel_size[type];
        if(type == WHEEL_TYPE_DAY || wsize >= remain)
        {
            slot = (u32)(wheel->cur + (s32)remain) % wsize;
            wheel->tlist[slot].Push(timer);
            break;
        }
        else
        {
            remain -= 1;
            remain /= wsize;
            type++;
        }
    } while(1);
}

void TimerMgr::Fire(TimeWheel *wheel, u64 tick)
{
    s32    ret;
    Timer     *timer;
    u32   size;

    if(++(wheel->cur) == wheel_size[wheel->type])
    {
        wheel->cur = 0;
    }

    if((size = wheel->tlist[wheel->cur].Size()) != 0)
    {
        for(u32 i = 0; i < size; ++i)
        {
            timer = (Timer*)wheel->tlist[wheel->cur].Pop();
            if(NULL == timer)
            {
                continue;
            }

            if(wheel->type == WHEEL_TYPE_SEC)
            {
                timer->is_in_proc_ = true;
                ret = timer->OnTimeout(tick);
                if(ret != 0)
                {
                    LogError() << "on timeout exec failed";
                }
                timer->is_in_proc_ = false;

                //对于执行过的定时器，从链表中删除定时器
                UnRegisterTimer(timer);

                //非永久执行的定时器，减少一次执行次数
                if(!timer->forever_)
                {
                    --timer->repeats_;
                }

                //如果还需要继续执行，重启计算超时时间
                if(timer->repeats_ > 0 || timer->forever_)
                {
                    timer->expire_ = CAL_EXPIRE(tick, timer->interval_);
                }
                //该定时器不需要再继续执行，销毁定时器
                else
                {
                    DeleteTimer(timer);
                    continue;
                }
            }
            Reg(timer, tick);
        }
    }

    if(wheel->cur + 1 == wheel_size[wheel->type] && wheel->type < WHEEL_TYPE_DAY)
    {
        Fire(&wheels_[wheel->type + 1], tick);
    }
}

u64 TimerMgr::RegisterTimer(u32 timeout, u32 interval, s32 repeats,
                            const s32 timer_func_id, const void *user_data,
                            s32 data_len)
{
    if (timer_func_id <= E_BASE_TIMER_FUNC_ID_INVALID ||
        timer_func_id >= MAX_TIMER_FUNC_ID_NUM)
    {
        LogError() << "invalid timer_func_id=" << timer_func_id;
        return INVALID_TIMER_ID;
    }

    if ((data_len > 0 && user_data == NULL) || data_len > TIMER_CB_DATA_MAX_LEN)
    {
        LogError() << "invalid input data (data len: "<<data_len<<")";
        return INVALID_TIMER_ID;
    }
    u64 timer_id = GenGUID();
    if (unlikely(timer_id == 0))
    {
        LogFatal() << "timer_id invalid";
        return INVALID_TIMER_ID;
    }
    auto it = timers_.find(timer_id);
    if (unlikely(it != timers_.end()))
    {
        LogFatal() << "timer_id="<<timer_id<<" is duplicate";
        return INVALID_TIMER_ID;
    }
    Timer *timer = TimerMem::CreateTimer();
    if(NULL == timer)
    {
        LogError() << "call Timer alloc failed";
        return INVALID_TIMER_ID;
    }
    timer->set_timer_id(timer_id);

    LogTrace() << "[obj create timer_node]" << _LogK(interval)
            << _LogK(timer_id) << _LogK(repeats) << _LogK(timer_func_id);

    // 向hashmap中插入新定时器
    auto ret = timers_.insert(std::make_pair(timer_id, timer));
    // 插入失败
    if(false == ret.second)
    {
        TimerMem::DestroyTimer(timer);
        LogDebug() << "[obj delete timer_node]"
            << _LogK(timer_id) << _LogK(repeats) << _LogK(timer_func_id);
        return INVALID_TIMER_ID;
    }

    timer->timeout_ = CommonUtil::Clamp(timeout, 1U, (u32)MAX_TIMEOUT);
    timer->interval_ = CommonUtil::Clamp(interval, 1U, (u32)MAX_TIMEOUT);
    timer->timer_func_id_ = timer_func_id;
    timer->repeats_ = repeats;
    timer->forever_ = (repeats == 0);
    timer->data_len_ = data_len;
    if (user_data != NULL)
    {
        memcpy(timer->data_, user_data, data_len);
    }

    u64 now = DateTime::GetNowSteadyMSec();

    if (NULL == tick_)
    {
        tick_  = &lasttick_;
        lasttick_ = now;
    }

    timer->expire_ = CAL_EXPIRE(now, timer->timeout_);

    Reg(timer, lasttick_);

    return timer_id;
}

u64 TimerMgr::RegisterTimer(u32 timeout, s32 repeats, const s32 timer_func_id,
                            const void *user_data, s32 data_len)
{
    return RegisterTimer(timeout, timeout, repeats, timer_func_id, user_data,
                         data_len);
}

s32 TimerMgr::GetRemainNum(u64 timer_id) const
{
    auto it = timers_.find(timer_id);
    if (it == timers_.end() || it->second == NULL)
    {
        LogDebug() << "can not find timer_id=" << timer_id;
        return 0;
    }
    s32 ret_num = it->second->forever_ ? -1 : it->second->repeats_;

	return ret_num;
}

u32 TimerMgr::GetRemainMs(u64 timer_id) const
{
    auto it = timers_.find(timer_id);
    if (it == timers_.end() || it->second == NULL)
    {
        LogDebug() << "can not find timer_id=" << timer_id;
        return 0;
    }
    u64 cur_ms = DateTime::GetNowSteadyMSec();
    if (cur_ms >= it->second->expire_)
    {
        return 0;
    }
    else
    {
        return it->second->expire_ - cur_ms;
    }
}

u64 TimerMgr::GetExpireMs(u64 timer_id) const
{
    auto it = timers_.find(timer_id);
    if (it == timers_.end() || it->second == NULL)
    {
        LogError() << "can not find timer_id=" << timer_id;
        return 0;
    }
    return it->second->expire_;
}


s32 TimerMgr::DestroyTimer(u64 timer_id)
{
    if(INVALID_TIMER_ID == timer_id)
    {
        LogError() << "invalid args.";
        return -1;
    }

    auto it = timers_.find(timer_id);
    if(it == timers_.end())
    {
        LogDebug() << "can not find timer_id=" << timer_id;
        return 0;
    }

    // 在定时器执行过程中只打标记
    if(it->second->is_in_proc_)
    {
        it->second->forever_ = false;
        it->second->repeats_ = 0;
    }
    else
    {
        DeleteTimer(it->second);
    }

    return 0;
}

void TimerMgr::DeleteTimer(Timer* timer)
{
    if(NULL == timer)
    {
        LogError() << "invalid args.";
        return;
    }

    u64 timer_id = timer->timer_id();

    if(timer->owner != NULL)
    {
        UnRegisterTimer(timer);
    }
    
    LogTrace() << "[obj create timer_node]"
            << _LogK(timer_id) << _LogKV("timer_func_id", timer->timer_func_id_);
    TimerMem::DestroyTimer(timer);

    timers_.erase(timer_id);
}


s32 TimerMgr::UnRegisterTimer(Timer *timer)
{
    if(NULL == timer || NULL == timer->owner)
    {
        return -1;
    }
    DoubLink::Remove(timer);
    return 0;
}


s32 TimeWheel::Init(s32 wheel_type)
{
    type = wheel_type;
    tlist = new DoubLink[wheel_size[type]];
    if (NULL == tlist)
    {
        LogError() << "init wheel list failed.";
        return -1;
    }

    cur = (type == WHEEL_TYPE_SEC) ? -1 : 0;

    return 0;
}

void TimeWheel::Destroy()
{
    if (tlist)
    {
        delete[] tlist;
    }
}

TimerMgr::TimerMgr()
{
    tick_ = NULL;
    lasttick_ = 0;
}

TimerMgr::~TimerMgr()
{

}

s32 TimerMgr::Init()
{
    for(u32 i = 0; i < WHEEL_TYPE_MAX; ++i)
    {
        s32 ret = wheels_[i].Init(i);
        if (ret != 0)
        {
            LogError() << "wheel <"<<i<<"> init failed.ret<"<<ret<<">.";
            return -1;
        }
    }

    return 0;
}

s32 TimerMgr::Destroy()
{
    for(u32 i = 0; i < WHEEL_TYPE_MAX; ++i)
    {
        wheels_[i].Destroy();
    }
    return 0;
}

namespace acm
{

void TimerWrapper::Clear()
{
    if (id_ != INVALID_TIMER_ID)
    {
        g_trans_server_ptr->timer_mgr()->DestroyTimer(id_);
        id_ = INVALID_TIMER_ID;
    }
}

} // namespace acm
