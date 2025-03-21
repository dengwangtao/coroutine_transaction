#ifndef COROUTINE_DEF_H
#define COROUTINE_DEF_H

#include "common_def.h"

using COROUTINE_FUNC = void(*)(void* param);


enum CoroutineStateDef
{
    E_COROUTINE_STATE_UNINIT = 0,
    E_COROUTINE_STATE_READY = 1,
    E_COROUTINE_STATE_RUN = 2,
    E_COROUTINE_STATE_SUSPEND = 3,
    E_COROUTINE_STATE_DEAD = 4, // 可以通过置成dead的方式，在主线程里定时清除销毁的协程
    E_COROUTINE_STATE_MAX
};

const s32 kDefaultStackSize = 1024 * 512;

#endif // COROUTINE_DEF_H
