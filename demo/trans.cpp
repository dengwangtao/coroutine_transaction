#include "trans.h"
#include "transaction/transaction_instance.h"



DemoTransaction::DemoTransaction()
    : Transaction(E_TRANSACTION_TYPE_DEMO, true)
{
    cmd_array_.push_back(&cmd1_);
    cmd_array_.push_back(&cmd2_);
    cmd_array_.push_back(&cmd3_);
}

s32 DemoTransaction::OnSuccess(TransactionInstance &inst)
{
    LogInfo() << _LogKV("inst", inst.id()) << " Success";
    return 0;
}


DemoTransaction2::DemoTransaction2()
    : Transaction(E_TRANSACTION_TYPE_DEMO_2, true)
{
    cmd_array_.push_back(&cmd3_);
    cmd_array_.push_back(&cmd2_);
    cmd_array_.push_back(&cmd1_);
}