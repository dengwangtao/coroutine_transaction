#include "cs_req_id_util.h"
#include "transaction.h"
#include "proto_base.pb.h"
#include "transaction/transaction_instance.h"

std::vector<uint32_t> cs_req_id_stack_{};

namespace cs_req_id_util
{

size_t Push(uint32_t cs_req_id)
{
    cs_req_id_stack_.emplace_back(cs_req_id);
    return cs_req_id_stack_.size();
}

void Pop(size_t seq)
{
    if (seq == 0 || seq != cs_req_id_stack_.size())
    {
        return;
    }
    cs_req_id_stack_.pop_back();
}

SetResult SetHead(CSHead& head)
{
    if (head.cs_req_id())
    {
        return SetResult::AlreadySet;
    }
    if (cs_req_id_stack_.empty())
    {
        return SetResult::NoReqCmd;
    }
    if (cs_req_id_stack_.back() == 0)
    {
        return SetResult::NoReqID;
    }
    // if (GetCSCmdType(head.cmd()) != CmdType::kCmdTypeRsp)
    // {
    //     return SetResult::NotRspCmd;
    // }
    head.set_cs_req_id(cs_req_id_stack_.back());
    return SetResult::OK;
}

SetResult SetHead(SSHead& head)
{
    if (head.cs_req_id())
    {
        return SetResult::AlreadySet;
    }
    if (cs_req_id_stack_.empty())
    {
        return SetResult::NoReqCmd;
    }
    if (cs_req_id_stack_.back() == 0)
    {
        return SetResult::NoReqID;
    }
    head.set_cs_req_id(cs_req_id_stack_.back());
    return SetResult::OK;
}

SetResult SetTranInst(TransactionInstance& inst)
{
    if (cs_req_id_stack_.empty())
    {
        return SetResult::NoReqCmd;
    }
    if (cs_req_id_stack_.back() == 0)
    {
        return SetResult::NoReqID;
    }
    inst.set_cs_req_id(cs_req_id_stack_.back());
    return SetResult::OK;
}

} // namespace cs_req_id_util
