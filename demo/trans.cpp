#include "trans.h"




DemoTransaction::DemoTransaction()
    : Transaction(E_TRANSACTION_TYPE_DEMO, true)
{
    cmd_array_.push_back(&cmd1_);
    cmd_array_.push_back(&cmd2_);
    cmd_array_.push_back(&cmd3_);
}