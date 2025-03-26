#include "transaction_server.h"
#include "coroutine_scheduler.h"


std::unique_ptr<TransactionServer> g_trans_server_ptr;


TransactionServer::TransactionServer()
    : scheduler_ptr_{ new CoroutineScheduler{} }
    , tran_mgr_ptr_ { new TranInstMgr{} }
    , timer_mgr_ptr_ { new TimerMgr{} }
{

}

TransactionServer::~TransactionServer()
{

}