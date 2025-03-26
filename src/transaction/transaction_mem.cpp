#include "transaction_mem.h"


TransactionInstance* TransactionMem::CreateTransactionInst(s32 type, u64 owner_id)
{
    return new TransactionInstance(type, owner_id);
}

void TransactionMem::DeleteTransactionInst(TransactionInstance* inst)
{
    delete inst;
}