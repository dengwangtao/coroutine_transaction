#pragma once
#ifndef SRC_TRANSACTION_TRANSACTION_MEM_H_
#define SRC_TRANSACTION_TRANSACTION_MEM_H_


#include "transaction_instance.h"

namespace TransactionMem
{
    TransactionInstance* CreateTransactionInst(s32 type, u64 owner_id);

    void DeleteTransactionInst(TransactionInstance* inst);
}



#endif // SRC_TRANSACTION_TRANSACTION_MEM_H_