#pragma once
#include <cstdint>
#include <cstddef>

class CSHead;
class SSHead;
class TransactionInstance;

namespace cs_req_id_util
{

size_t Push(uint32_t cs_req_id);
void Pop(size_t seq);

enum class SetResult
{
    OK = 0,
    AlreadySet = 1, // 已经设置过cs_id，不重复设置
    NotRspCmd = 2,  // 不是RspCmd，一般是Ntf，此时不附带cs_req_id
    NoReqCmd = 3,   // 没有ReqCmd的情况下下发RspCmd
    NoReqID = 4,    // Req里没有设置cs_req_id
};

SetResult SetHead(CSHead& head);
SetResult SetHead(SSHead& head);
SetResult SetTranInst(TransactionInstance& inst);

}
