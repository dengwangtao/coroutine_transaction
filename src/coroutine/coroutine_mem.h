#pragma once

#ifndef SRC_TRANSACTION_COROUTINE_MEM_H_
#define SRC_TRANSACTION_COROUTINE_MEM_H_


#include "coroutine.h"

namespace CoroutineMem
{
    CoroutineImpl* CreateCoroutine();
    void DestroyCoroutine(CoroutineImpl* co);
}


#endif // SRC_TRANSACTION_COROUTINE_MEM_H_