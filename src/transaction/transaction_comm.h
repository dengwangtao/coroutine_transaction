#pragma once
#ifndef SRC_TRANSACTION_TRANSACTION_COMM_H_
#define SRC_TRANSACTION_TRANSACTION_COMM_H_



#include "common_def.h"
#include "command.h"




// 事务事件参数
enum TransactionEventType
{
    E_TRANSACTION_EVENT_TYPE_INVALID = 0, // 无效
    E_TRANSACTION_EVENT_TYPE_TIMEOUT = 1, // 超时
    E_TRANSACTION_EVENT_TYPE_ABORT   = 2, // 强制停止协程
};



struct TransactionEventArg
{
    void* get_void_ptr()
    {
        return msg;
    }
    void* msg = nullptr;
};



#endif // SRC_TRANSACTION_TRANSACTION_COMM_H_