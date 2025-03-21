#pragma once

#ifndef SRC_TRANSACTION_TRANSACTION_H_
#define SRC_TRANSACTION_TRANSACTION_H_

#include <iostream>
#include "transaction_comm.h"


// Transaction不能记录状态, 只能记录在TransactionInst里面. 因此没必要放在共享
// 内存中, 不然resume反倒麻烦
class Transaction
{
    friend TransactionInstance;
    static constexpr int kDefaultCmdCount = 20;


public:
    Transaction(s32 type, bool is_need_undo);
    virtual ~Transaction();

    // If this function succeeds, the memory of inst is taken over. So we use a
    // pointer param here.
    s32 Start(TransactionInstance *inst);

    s32 Resume(TransactionInstance &inst);

    // Must be called in the main routine so the business layer can write
    // logic easily.
    void HandleResult(TransactionInstance &inst);

    s32 type() const { return type_; }

    const char* GetName() const;

    const char* GetCmdName(int index) const;

private:
    // Never change the accessor of these functions.
    s32 RealStart(TransactionInstance &inst);
    s32 RealResume(TransactionInstance &inst);

    void RunCommandOnInstance(TransactionInstance &inst, int index);

    s32 Undo(TransactionInstance &inst);

    virtual s32 OnStart(TransactionInstance& inst) { return 0; }

    // The following functions are called in the main routine instead of the
    // work coroutine.
    virtual s32 OnSuccess(TransactionInstance &inst) { return 0; }
    virtual s32 OnFail(TransactionInstance &inst) { return 0; }
    virtual s32 OnAbort(TransactionInstance& inst) { return OnFail(inst); }
    virtual void Finally(TransactionInstance& inst) { (void) inst; }

    static void TransactionCoroutineEntry(void* param, bool is_resume);
    static void TransactionCoroutineBootEntry(void* param);
    static void TransactionCoroutineResumeEntry(void* param);

protected:
    std::array<Command*, kDefaultCmdCount> cmd_array_;
    const s32 type_;
    const bool is_need_undo_; // 执行成功是否需要执行undo流程
    mutable char demangled_cls_name_[kMaxDemangledClassNameSize];

    /// 这个地方不能定义任何和具体事务相关的变量，和具体事务相关的变量必须定义在TransactionInstance中
    // 否则会因为协程切换的原因，被其他实例改写，造成逻辑错误
};

#endif // SRC_TRANSACTION_TRANSACTION_H