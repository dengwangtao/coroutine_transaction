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

private:
    TestCmd cmd1_;
    TestCmd cmd2_;
    TestCmd cmd3_;
};

#endif //DEMO_TRANS_H