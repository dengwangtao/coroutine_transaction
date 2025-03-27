#include "trans.h"




DemoTransaction::DemoTransaction()
    : Transaction(E_TRANSACTION_TYPE_DEMO, true)
{
    cmd_array_.push_back(&cmd1_);
    cmd_array_.push_back(&cmd2_);
    cmd_array_.push_back(&cmd3_);
}


DemoTransaction2::DemoTransaction2()
    : Transaction(E_TRANSACTION_TYPE_DEMO_2, true)
{
    cmd_array_.push_back(&cmd3_);
    cmd_array_.push_back(&cmd2_);
    cmd_array_.push_back(&cmd1_);
}