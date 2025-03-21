#pragma once
#ifndef SRC_TRANSACTION_TRANSACTION_COMM_H_
#define SRC_TRANSACTION_TRANSACTION_COMM_H_



#include "common_def.h"
#include "command.h"

#define USE_ABSL_VARIANT
#ifdef USE_ABSL_VARIANT
    #include "absl/types/variant.h"
    #define STD absl
#endif

class SSHead;

namespace google
{
    namespace protobuf
    {
        class Message;
    } // protobuf
} // google


// 事务事件参数
enum TransactionEventType
{
    E_TRANSACTION_EVENT_TYPE_INVALID = 0, // 无效
    E_TRANSACTION_EVENT_TYPE_TIMEOUT = 1, // 超时
    E_TRANSACTION_EVENT_TYPE_ABORT   = 2, // 强制停止协程
};



struct TransactionEventArg
{
    struct ServerProtoMessage
    {
        const SSHead *head;
        const google::protobuf::Message *body;
    };

    void* get_void_ptr()
    {
        void **ptr = STD::get_if<void*>(&msg);
        return ptr ? *ptr : nullptr;
    }

    const SSHead* get_proto_head() const
    {
        auto *spm = STD::get_if<ServerProtoMessage>(&msg);
        return spm ? spm->head : nullptr;
    }

    template <typename T>
    const T* get_proto_body() const
    {
        auto *spm = STD::get_if<ServerProtoMessage>(&msg);
        return spm ? dynamic_cast<const T*>(spm->body) : nullptr;
    }

    STD::variant<void*, ServerProtoMessage> msg;
};



#endif // SRC_TRANSACTION_TRANSACTION_COMM_H_