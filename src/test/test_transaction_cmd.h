#pragma once

#include "transaction_server.h"
#include "trans_type.pb.h"
#include "transaction.h"
#include "trans_type.pb.h"
#include "transaction_instance.h"

// =================== 事务流程Command ====================


class TestCmd : public Command
{
public:
    TestCmd() : Command(E_TRAN_EVENT_TYPE_TEST)
    {
    }
    s32 Do(TransactionInstance &inst) override
    {
        LogInfo() << "TestCmd::Do" << _LogKV("inst", inst.id()) << _LogKV("inst_owner", inst.owner_id());
        return 0;
    }
    s32 OnRecvMsgEvent(TransactionInstance &inst, s32 event_id) override
    {
        LogInfo() << "TestCmd::OnRecvMsgEvent";
        return 0;
    }
};

// =============================================================



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
    EnterGameTransaction() : Transaction(E_TRANSACTION_TYPE_ENTER_GAME, true)
    {
        cmd_array_.push_back(&cmd1_);
        cmd_array_.push_back(&cmd2_);
        cmd_array_.push_back(&cmd3_);
    }

private:
    
    TestCmd cmd1_;
    TestCmd cmd2_;
    TestCmd cmd3_;
};

// =============================================================



// =================== 事务服务器 ====================

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