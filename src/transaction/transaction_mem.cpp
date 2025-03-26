#include "transaction/transaction_mem.h"
#include "transaction/transaction_instance.h"


void DeleteTransactionInst(TransactionInstance* inst)
{
    delete inst;
}