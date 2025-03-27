
#pragma once
#ifndef DEMO_TRANS_SERVER_H
#define DEMO_TRANS_SERVER_H

#include "transaction_server.h"
#include "trans_type.pb.h"

// =================== 事务服务器 ====================

class DemoTransactionServer : public TransactionServer, public Singleton<DemoTransactionServer>
{

public:
    s32 Init() override;

    Transaction *GetTranByType(s32 type) const override;
};


#endif // DEMO_TRANS_SERVER_H