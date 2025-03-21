#include <iostream>
#include "singleton.h"
#include "proto_base.pb.h"
#include "common_def.h"
#include "boost_context.h"
#include "fstream"


// int main()
// {
//     std::cout << "Hello, World!" << std::endl;
//     SSHead head;
//     head.set_cmd(1);

//     std::cout << head.DebugString() << std::endl;


//     LogError() << "error";
//     LogInfo() << "info";
//     LogTrace() << "trace";
//     LogFatal() << "fatal";
    

//     return 0;
// }


#define CO_COUNT 50
#define CO_STACK_SIZE 80000


// 让boost_context支持4k对齐
#define ALIGN(stack) \
    (stack - ((unsigned long long)stack % 4096) + 4096)


struct Context
{
    BoostContext ctx = nullptr;
    unsigned char stack[CO_STACK_SIZE]; // 协程栈大小
};

Context g_cos[CO_COUNT];
Context main_co;

void task(void* param)
{
    LogDebug() << "ss";
    // 都跳回主协程
    JumpContext((BoostContext*)param, main_co.ctx, param, true);
}

void star_schedule()
{
    for (int i = 0; i < CO_COUNT; i ++)
    {
        auto& co = g_cos[i];
        co.ctx = MakeContext(ALIGN(co.stack) + (CO_STACK_SIZE / 4096 - 2) * 4096, (CO_STACK_SIZE/4096 -2)*4096, &task);
    }
    main_co.ctx = NULL;
    for (auto& co : g_cos)
    {
        LogDebug() << "star schedule swap";
        // 跳回工作协程
        JumpContext(&main_co.ctx, co.ctx, &co.ctx, true);
    }
    LogDebug() << "one loop finish ...";
}

void stress(void* param)
{
    while (true)
    {
        JumpContext((BoostContext*)param, main_co.ctx, NULL, true);
    }
}
void stress_schedule()
{
    Context worker;
    worker.ctx = MakeContext(ALIGN(worker.stack) + (CO_STACK_SIZE / 4096 - 2) * 4096, (CO_STACK_SIZE/4096 -2)*4096, &stress);
    auto begin_time = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000*10000; ++i)
    {
        JumpContext(&main_co.ctx, worker.ctx, &worker.ctx, true);
    }
    auto end_time = std::chrono::steady_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - begin_time).count();
    LogDebug() << "stress schedule use :<"<<duration<<">s";
    LogDebug() << "one loop finish ...";
}

void real_stress(void* param)
{
    static int count = 0;
    while (true)
    {
        // 打开文件, 写入数据
        std::fstream file("test.txt", std::ios::out | std::ios::app);
        file << count ++ << std::endl;
        file.close();

        JumpContext((BoostContext*)param, main_co.ctx, NULL, true);
    }
}

void real_stress_schedule()
{
    for (int i = 0; i < CO_COUNT; i ++)
    {
        auto& co = g_cos[i];
        co.ctx = MakeContext(ALIGN(co.stack) + (CO_STACK_SIZE / 4096 - 2) * 4096, (CO_STACK_SIZE/4096 -2)*4096, &real_stress);
    }

    auto begin_time = std::chrono::steady_clock::now();

    for (int i = 0; i < 10*10000; ++i)
    {
        for (auto& co : g_cos)
        {
            JumpContext(&main_co.ctx, co.ctx, &co.ctx, true);
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - begin_time).count();
    LogDebug() << "real stress schedule use :<"<<duration<<">s";
    LogDebug() << "100'000 loop finish ...";
}

int main(int argc, char * argv[])
{
    LogInfo() << "begin main call ...";
    LogInfo() << "begin star schedule call ...";
    star_schedule();
    LogInfo() << "end star schedule call ...";
    LogInfo() << "begin ring schedule call ...";
    LogInfo() << "end ring schedule call ...";
    LogInfo() << "=============================";
    LogInfo() << "begin stress schedule call ...";
    stress_schedule();
    LogInfo() << "end stress schedule call ...";
    LogInfo() << "end main call ...";
    LogInfo() << "=============================";
    LogInfo() << "begin real stress schedule call ...";
    real_stress_schedule();
    LogInfo() << "end real stress schedule call ...";
    LogInfo() << "end main call ...";
    return 0;
}