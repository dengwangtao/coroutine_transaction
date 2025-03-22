/*
            Copyright Oliver Kowalke 2009.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

/****************************************************************************************
 *                                                                                      *
 *  ----------------------------------------------------------------------------------  *
 *  |    0    |    1    |    2    |    3    |    4     |    5    |    6    |    7    |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x0   |   0x4   |   0x8   |   0xc   |   0x10   |   0x14  |   0x18  |   0x1c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  | fc_mxcsr|fc_x87_cw|        R12        |         R13        |        R14        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    8    |    9    |   10    |   11    |    12    |    13   |    14   |    15   |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x20  |   0x24  |   0x28  |  0x2c   |   0x30   |   0x34  |   0x38  |   0x3c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        R15        |        RBX        |         RBP        |        RIP        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    16   |   17    |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x40  |   0x44  |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        EXIT       |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *                                                                                      *
 ****************************************************************************************/

.text
.globl MakeContext
.type MakeContext,@function
.align 16
MakeContext:
    /* first arg of MakeContext() == top of context-stack */
    movq  %rdi, %rax    ; 第一个参数 rdi 是用户提供的栈顶地址，存入 rax

    /* shift address in RAX to lower 16 byte boundary */
    andq  $-16, %rax    ; 将 rax 向下对齐到 16 字节边界

    /* reserve space for context-data on context-stack */
    /* size for fc_mxcsr .. RIP + return-address for context-function */
    /* on context-function entry: (RSP -0x8) % 16 == 0 */
    leaq  -0x48(%rax), %rax ; 从 rax 中减去 0x48（72字节），预留上下文空间

    /* third arg of MakeContext() == address of context-function */
    movq  %rdx, 0x38(%rax)  ; 第三个参数 rdx 是协程函数地址，存入 0x38 偏移处（RIP 位置）

    /* save MMX control- and status-word */
    stmxcsr  (%rax) ; 保存 MXCSR（SIMD 控制状态）到 0x0 偏移处（fc_mxcsr）
    /* save x87 control-word */
    fnstcw   0x4(%rax)  ; 保存 x87 控制字到 0x4 偏移处（fc_x87_cw）

    /* compute abs address of label finish */
    leaq  finish(%rip), %rcx    ; 计算 finish 标签的绝对地址，存入 rcx
    /* save address of finish as return-address for context-function */
    /* will be entered after context-function returns */
    movq  %rcx, 0x40(%rax)  ; 将 finish 地址存入 0x40 偏移处（EXIT 字段）
    ret /* return pointer to context-data */ ; 返回 rax（指向初始化后的上下文数据）

finish:
    /* exit code is zero */
    xorq  %rdi, %rdi    ; 将退出代码设为 0（rdi=0）
    /* exit application */
    call  _exit@PLT     ; 调用 exit(0) 结束程序
    hlt   ; 停机指令（通常不会执行）
.size MakeContext,.-MakeContext

/* Mark that we don't need executable stack. */
.section .note.GNU-stack,"",%progbits
