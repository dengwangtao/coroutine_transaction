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

.text   ; 表示后续代码属于 .text 段（存放可执行代码）
.globl JumpContext   ; 声明 JumpContext 为全局函数，其他文件可调用
.type JumpContext,@function  ; 标记 JumpContext 是一个函数
.align 16   ; 代码按 16 字节对齐（优化 CPU 执行速度）
JumpContext:    ; 函数入口标签
    pushq  %rbp  /* save RBP */ ; 保存基址指针寄存器（调用函数时必须保留的寄存器）
    pushq  %rbx  /* save RBX */ ; 保存基址寄存器
    pushq  %r15  /* save R15 */ ; 保存通用寄存器 R15
    pushq  %r14  /* save R14 */ ; 保存通用寄存器 R14
    pushq  %r13  /* save R13 */ ; 保存通用寄存器 R13
    pushq  %r12  /* save R12 */ ; 保存通用寄存器 R12

    /* prepare stack for FPU */
    leaq  -0x8(%rsp), %rsp      ; 将栈指针 rsp 减 8 字节（预留 8 字节空间）

    /* test for flag preserve_fpu */
    cmp  $0, %rcx               ; 比较 rcx（第三个参数）是否为 0
    je  1f                      ; 如果 rcx=0（不保存 FPU），跳转到标签 1

    /* save MMX control- and status-word */
    stmxcsr  (%rsp)             ; 保存 SIMD 控制状态（MXCSR）到栈顶
    /* save x87 control-word */
    fnstcw   0x4(%rsp)          ; 保存 x87 浮点控制字到栈顶+4 的位置

1:  ; 标签 1（跳过 FPU 保存的分支）
    /* store RSP (pointing to context-data) in RDI */
    movq  %rsp, (%rdi)      ; 将当前栈指针 rsp 保存到 rdi 指向的内存地址


    /* restore RSP (pointing to context-data) from RSI */
    movq  %rsi, %rsp        ; 将 rsi 的值（新上下文的栈指针）加载到 rsp

    /* test for flag preserve_fpu */
    cmp  $0, %rcx           ; 再次检查 rcx（是否恢复 FPU）
    je  2f                  ; 如果 rcx=0，跳转到标签 2

    /* restore MMX control- and status-word */
    ldmxcsr  (%rsp)         ; 从栈顶加载 MXCSR 寄存器
    /* restore x87 control-word */
    fldcw  0x4(%rsp)        ; 从栈顶+4 加载 x87 控制字

2:  ; 标签 2（跳过 FPU 恢复的分支）
    /* prepare stack for FPU */
    leaq  0x8(%rsp), %rsp       ; 将栈指针 rsp 加 8 字节（释放之前预留的空间）

    popq  %r12  /* restrore R12 */  ; 恢复 R12
    popq  %r13  /* restrore R13 */  ; 恢复 R13
    popq  %r14  /* restrore R14 */  ; 恢复 R14
    popq  %r15  /* restrore R15 */  ; 恢复 R15
    popq  %rbx  /* restrore RBX */  ; 恢复 RBX
    popq  %rbp  /* restrore RBP */  ; 恢复 RBP

    /* restore return-address */
    popq  %r8           ; 将栈顶的值（新上下文的 RIP）弹出到 r8

    /* use third arg as return-value after jump */
    movq  %rdx, %rax    ; 将 rdx（第三个参数）的值赋给 rax（返回值）
    /* use third arg as first arg in context function */
    movq  %rdx, %rdi    ; 将 rdx 赋给 rdi（作为新上下文函数的第一个参数）

    /* indirect jump to context */
    jmp  *%r8           ; 跳转到 r8 中存储的地址（新上下文的入口点）
.size JumpContext,.-JumpContext

/* Mark that we don't need executable stack.  */
.section .note.GNU-stack,"",%progbits
