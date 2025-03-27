#pragma once
#ifndef DEMO_TRANS_H
#define DEMO_TRANS_H

#include "transaction.h"
#include "transaction_instance.h"
#include "proto/trans_type.pb.h"
#include "cmds.h"
#include "singleton.h"

// ==================== 事务类型定义 ====================


class DemoTransaction : public Transaction, public Singleton<DemoTransaction>
{
public:
    DemoTransaction();

    s32 OnSuccess(TransactionInstance &inst) override;

private:
    TestCmd cmd1_;
    TestCmd2 cmd2_;
    TestCmd3 cmd3_;
};



class DemoTransaction2 : public Transaction, public Singleton<DemoTransaction2>
{
public:
    DemoTransaction2();

private:
    TestCmd cmd1_;
    TestCmd2 cmd2_;
    TestCmd3 cmd3_;
};

#endif //DEMO_TRANS_H