#pragma once
#ifndef SRC_TRANSACTION_TRANSACTION_SERVER_H_
#define SRC_TRANSACTION_TRANSACTION_SERVER_H_

#include "common_def.h"

class Transaction;


class TransactionServer
{
public:
    // 只有用到了事务且需要resume的server会需要, 因此提供了默认实现
    virtual Transaction* GetTranByType(s32 type) const
    {
        return nullptr;
    }

};


// TransactionServer* g_trans_server = nullptr;
std::unique_ptr<TransactionServer> g_trans_server_ptr;

#endif // SRC_TRANSACTION_TRANSACTION_SERVER_H_