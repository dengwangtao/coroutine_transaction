#pragma once

#include <iostream>
#include <memory>
#include "singleton.h"
#include "proto_base.pb.h"
#include "common_def.h"
#include "boost_context.h"
#include "fstream"
#include "transaction/transaction_server.h"
#include "trans_type.pb.h"
#include "transaction/transaction.h"

// ==================== 事务类型定义 ====================

class QueryAccountTransaction : public Transaction, public Singleton<QueryAccountTransaction>
{
public:
    QueryAccountTransaction() : Transaction(E_TRANSACTION_TYPE_QUERY_ACCOUNT, true) {}
};

class CreateAccountTransaction : public Transaction, public Singleton<CreateAccountTransaction>
{
public:
    CreateAccountTransaction() : Transaction(E_TRANSACTION_TYPE_CREATE_ACCOUNT, true) {}
};

class CreateRoleTransaction : public Transaction, public Singleton<CreateRoleTransaction>
{
public:
    CreateRoleTransaction() : Transaction(E_TRANSACTION_TYPE_CREATE_ROLE, true) {}
};

class DeleteRoleTransaction : public Transaction, public Singleton<DeleteRoleTransaction>
{
public:
    DeleteRoleTransaction() : Transaction(E_TRANSACTION_TYPE_DEL_ROLE, true) {}
};

class EnumRoleTransaction : public Transaction, public Singleton<EnumRoleTransaction>
{
public:
    EnumRoleTransaction() : Transaction(E_TRANSACTION_TYPE_ENUM_ROLE, true) {}
};

class EnterGameTransaction : public Transaction, public Singleton<EnterGameTransaction>
{
public:
    EnterGameTransaction() : Transaction(E_TRANSACTION_TYPE_ENTER_GAME, true) {}
};

// =============================================================

class TestTransactionServer : public TransactionServer
{

public:
    Transaction *GetTranByType(s32 type) const override
    {
        switch (type)
        {
        case E_TRANSACTION_TYPE_QUERY_ACCOUNT:
            return &QueryAccountTransaction::Instance();

        case E_TRANSACTION_TYPE_CREATE_ACCOUNT:
            return &CreateAccountTransaction::Instance();

        case E_TRANSACTION_TYPE_CREATE_ROLE:
            return &CreateRoleTransaction::Instance();

        case E_TRANSACTION_TYPE_DEL_ROLE:
            return &DeleteRoleTransaction::Instance();

        case E_TRANSACTION_TYPE_ENUM_ROLE:
            return &EnumRoleTransaction::Instance();

        case E_TRANSACTION_TYPE_ENTER_GAME:
            return &EnterGameTransaction::Instance();

        default:
            LogError() << "unknown tran type=" << type;
            break;
        }

        return nullptr;
    }
};

int main(int argc, char *argv[])
{
    LogInfo() << "begin main call ...";

    g_trans_server_ptr = std::unique_ptr<TestTransactionServer>(new TestTransactionServer());

    return 0;
}