#pragma once

#ifndef BOOST_CONTEXT_H
#define BOOST_CONTEXT_H


using BoostContext = void*;
using BoostCoroutineFunc = void (*)(void* param);

/**
 * @brief 跳转上下文
 * @param ofc 原上下文
 * @param nfc 目标上下文
 * @param param 参数(进入协程函数时传递的参数)
 * @param preserve_fpu 是否保留浮点寄存器
 * @return 0 成功
 */
extern "C"
s64 JumpContext(BoostContext* ofc, BoostContext nfc, void* param, bool preserve_fpu = false);



/**
 * @brief 创建上下文
 * @param stack 栈地址
 * @param size 栈大小 (asm内部并没有使用)
 * @param func 函数
 */
extern "C"
BoostContext MakeContext(void* stack, size_t size, BoostCoroutineFunc func);


#endif // BOOST_CONTEXT_H