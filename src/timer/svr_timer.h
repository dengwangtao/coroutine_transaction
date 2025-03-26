#pragma once
#ifndef _TTIMER_H
#define _TTIMER_H

#include <stdint.h>
#include <stdio.h>
#include <cstring>
#include <unordered_map>
#include <type_traits>
#include "singleton.h"
#include "linklist.h"

/*
*  定时器支持的最大超时值(毫秒),如果传入的超时值大于MAX_TIMEOUT
*  超时值将被设置为MAX_TIMEOUT
*/

#define MAX_TIMEOUT          (1000*3600*24-1)
#define TIMER_CB_DATA_MAX_LEN 128
#define INVALID_TIMER_ID      0ULL

//定时器回调函数
#define MAX_TIMER_FUNC_ID_NUM (64)
typedef void(*TIMEOUTCBFUNC)(u64 timer_id, void *data, size_t data_len);

class TimerMgr;

enum BASE_TIMEER_FUNC_ID
{
    E_BASE_TIMER_FUNC_ID_INVALID = 0,
    E_BASE_TIMER_FUNC_ID_TRANSACTION_ON_TIMEOUT,    // 事务超时
    // stateless
    //TIMER_FUNC_ID_STATELESS_BASE_ASYNCOP_TIMEOUT    = 4001, // 用于无状态服务器的事务等待超时检测,请将此定义移动到对应服务器所在文件中，参考其他定时器函数ID
    E_BASE_TIMER_FUNC_ID_MAX,
};

class Timer : public DoubLinkNode
{
    friend class TimerMgr;
public:
    /*
    *   返回0定时器重新注册，以实现重复定时器
    */
    s32 OnTimeout(u64 curms);

    Timer()
        : timeout_(0),
          interval_(0),
          expire_(0),
          repeats_(0),
          timer_func_id_(0),
          data_len_(0),
          forever_(false),
          is_in_proc_(false)
    {
        memset(data_, 0, sizeof(data_));
    }

    ~Timer();

    static void ClearTimerFunc()
    {
        memset(timer_func_array, 0, sizeof(timer_func_array));
    }

    static s32 RegisterTimerFunc(u32 timer_func_id, TIMEOUTCBFUNC func);

    void set_timer_id(u64 timer_id)
    {
        timer_id_ = timer_id;
    }
    u64 timer_id() const
    {
        return timer_id_;
    }

private:
    Timer(const Timer &o);
    Timer& operator = (const Timer &o);

private:
    u64 timer_id_ = 0;
    char data_[TIMER_CB_DATA_MAX_LEN];
    u64  timeout_;
    u64  interval_;
    u64  expire_;
    s32  repeats_;   //执行次数
    u32  timer_func_id_; // 函数ID
    s32  data_len_;
    bool forever_;  //是否为永久定时器
    bool is_in_proc_; // 是否在处理定时器函数中
private:
    static TIMEOUTCBFUNC timer_func_array[MAX_TIMER_FUNC_ID_NUM];
};

enum WHEEL_TYPE
{
    WHEEL_TYPE_SEC = 0,
    WHEEL_TYPE_HOUR = 1,
    WHEEL_TYPE_DAY = 2,
    WHEEL_TYPE_MAX
};

struct TimeWheel
{
    s32      type = 0;
    s32      cur = 0;
    DoubLink  *tlist = nullptr;
    s32 Init(s32 type);
    void Destroy();
};

class TimerMgr
{
public:
    TimerMgr();
    ~TimerMgr();

public:
    s32 Init();
    s32 Destroy();

    template <class DataType>
    u64 RegisterTimer(u32 timeout, s32 repeats, s32 timer_func_id, const DataType* cb)
    {
        static_assert(sizeof(DataType) <= size_t(TIMER_CB_DATA_MAX_LEN),
                      "Timer user data too large");
        return RegisterTimer(timeout, repeats, timer_func_id, cb, sizeof(*cb));
    }

    template <class DataType>
    u64 RegisterTimer(u32 timeout, u32 interval, s32 repeats, s32 timer_func_id, const DataType* cb)
    {
        static_assert(sizeof(DataType) <= size_t(TIMER_CB_DATA_MAX_LEN),
                      "Timer user data too large");
        return RegisterTimer(timeout, interval, repeats, timer_func_id, cb, sizeof(*cb));
    }

    // 可重入的删除定时器的函数。针对定时器正在执行，和定时器已经执行完毕，会做不同的操作
    s32 DestroyTimer(u64 timer_id);

    //获得计时器剩余的次数
    s32 GetRemainNum(u64 timer_id) const;

    //获得计时器剩余的ms
    u32 GetRemainMs(u64 timer_id) const;

    //获得计时器过期时间
    u64 GetExpireMs(u64 timer_id) const;

    void Proc();

#ifdef UNIT_TEST
    void set_lasttick(u64 tick) { lasttick_ = tick; }
    void set_tick(u64 tick) { lasttick_ = tick; }
#endif // UNIT_TEST

private:
    /**
    * @brief 注册一个定时器
    * @param timeout 超时时间ms
    * @param repeats 重复次数，0为永久定时器
    * @param timeout_func 回调函数
    * @param user_data 执行参数
    * @param data_len 参数长度
    */
    u64 RegisterTimer(u32 timeout, s32 repeats, const s32 timer_func_id, const void* user_data, s32 data_len);

    /**
     * @brief 注册一个定时器
     * @param timeout 第一次超时时间
     * @param interval 后续的重复间隔
     * @param repeats 重复次数，0为永久定时器
     * @param timeout_func 回调函数
     * @param user_data 执行参数
     * @param data_len 参数长度
     */
    u64 RegisterTimer(u32 timeout, u32 interval, s32 repeats,
                      s32 timer_func_id, const void *user_data, s32 data_len);

    /**
    @brief 删除定时器
    @param[in] timer_id -- 定时器id
    @retval 0 -- 成功
    @retval -1 -- 失败
    */
    void DeleteTimer(Timer* timer);

    void Reg(Timer *timer, u64 tick);

    void Fire(TimeWheel *wheel, u64 tick);

    s32 UnRegisterTimer(Timer *timer);

    TimerMgr(const TimerMgr &other);
    TimerMgr& operator = (const TimerMgr &other);

private:
    u64    *tick_;
    u64     lasttick_;
    TimeWheel wheels_[WHEEL_TYPE_MAX];
    std::unordered_map<u64, Timer*> timers_;
};

namespace acm
{

class TimerWrapper
{
public:
    explicit TimerWrapper(u64 id = INVALID_TIMER_ID) : id_ { id }
    {
    }

    ~TimerWrapper()
    {
        Clear();
    }

    TimerWrapper(TimerWrapper &&other)
    {
        id_ = other.release();
    }

    operator bool() const { return id_ != INVALID_TIMER_ID; }

    TimerWrapper(const TimerWrapper &) = delete;
    TimerWrapper& operator=(const TimerWrapper &) = delete;

    u64 get() const { return id_; }

    void reset(u64 id)
    {
        Clear();
        id_ = id;
    }

    u64 release()
    {
        auto id = id_;
        id_ = INVALID_TIMER_ID;
        return id;
    }

    void cancel()
    {
        Clear();
    }

private:
    void Clear();

    u64 id_;
};

} // namespace acm

#endif
