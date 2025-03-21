#include "coroutine_mem.h"


CoroutineImpl* CoroutineMem::CreateCoroutine()
{
    return new CoroutineImpl();
}

void CoroutineMem::DestroyCoroutine(CoroutineImpl* co)
{
    if (co)
    {
        delete co;
    }
}