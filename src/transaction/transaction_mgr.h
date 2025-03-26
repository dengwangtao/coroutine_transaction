#pragma once
#ifndef SRC_TRANSACTION_TRANSACTION_MGR_H_
#define SRC_TRANSACTION_TRANSACTION_MGR_H_

#include "transaction.h"
#include "singleton.h"

class TransactionInstance;


class TranInstMgr
{
public:
    s32 AddTranInst(TransactionInstance* inst);
    void Update();

    s32 Resume();

    // 为了不可预知的后果, 在调用本函数之后, 调用者不应当再操作tran_inst.
    s32 MarkTranInstDelayDestroy(TransactionInstance* tran_inst);

    void RemoveTranInst(u64 id) { trans_map_.erase(id); }

    int GetTranCount() const { return trans_map_.size(); }

    int GetUnfinishedTranCount() const { return trans_map_.size() + destroy_list_.size(); }

    TransactionInstance* GetTranInst(u64 id) const
    {
        auto iter = trans_map_.find(id);
        return (iter != trans_map_.end() ? iter->second : nullptr);
    }
private:
    std::map<u64, TransactionInstance*> trans_map_;
    std::set<TransactionInstance*> destroy_list_;
};

#endif // SRC_TRANSACTION_TRANSACTION_MGR_H_