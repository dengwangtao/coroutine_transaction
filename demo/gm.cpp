#include "gm.h"
#include "transaction_server.h"
#include "trans.h"
#include "utils.h"

#pragma region GM


s32 GMStart(u64 peer_id, const GMParam& param, std::string& out_str)
{
    u32 tran_type = Utils::strto<u32>(param[1]);

    if (TransactionType_IsValid(tran_type) == false)
    {
        LogError() << "invalid transaction type: " << tran_type;
        out_str = "invalid transaction type: " + std::to_string(tran_type);
        return -1;
    }

    auto tran = g_trans_server_ptr->GetTranByType(tran_type);
    if (tran == nullptr)
    {
        LogError() << "transaction type not found: " << tran_type;
        out_str = "transaction type not found: " + std::to_string(tran_type);
        return -2;
    }
    
    TransactionInstance* tran_inst = nullptr;
    g_trans_server_ptr->StartCommonTransaction(
        *tran,
        peer_id,
        &tran_inst
    );

    if (tran_inst == nullptr)
    {
        LogError() << "transaction instance create failed";
        return -3;
    }

    out_str = "start transaction success, transaction id: " + std::to_string(tran_inst->id());
    return 0;
}


s32 GMInvoke(u64 peer_id, const GMParam& param, std::string& out_str)
{
    u64 tran_id = Utils::strto<u64>(param[1]);
    s32 event_id = Utils::strto<s32>(param[2]);

    LogInfo() << "GMInvoke: " << tran_id << " " << event_id;

    auto* tran_inst = g_trans_server_ptr->tran_mgr()->GetTranInst(tran_id);
    if (! tran_inst)
    {
        LogError() << "transaction instance " << tran_id <<" not found";
        out_str = "transaction instance " + std::to_string(tran_id) + " not found";
        return -1;
    }
    if (TranEventType_IsValid(event_id) == false)
    {
        LogError() << "invalid event id: " << event_id;
        out_str = "invalid event id: " + std::to_string(event_id);
        return -2;
    }
    s32 ret = g_trans_server_ptr->SendMsgEventToTran(tran_id, event_id, &peer_id);

    return ret;
}

#pragma endregion GM





s32 RegisterAllGM()
{
    auto& mgr = GMMgr::Instance();
    mgr.RegisterGM("start", GMFuncInfo(1, "", GMStart)); // 开始一个事务
    mgr.RegisterGM("invoke", GMFuncInfo(2, "", GMInvoke)); // 触发一个事务
    return 0;
}





s32 GMMgr::RegisterGM(const std::string& name, const GMFuncInfo& func_info)
{
    if (gm_map_.find(name)!= gm_map_.end())
    {
        LogError() << "GM already registered: " << name;
        return -1;
    }
    gm_map_[name] = func_info;
    return 0;
}

s32 GMMgr::CallGM(u64 peer_id, const std::string& token, std::string* out_str)
{
    std::stringstream ss(token);
    GMParam param;
    std::string one;
    while (ss >> one)
    {
        param.push_back(one);
    }
    if (param.empty())
    {
        LogError() << "GM param is empty";
        return -1;
    }


    auto it = gm_map_.find(param[0]);
    if (it == gm_map_.end())
    {
        LogError() << "GM not found: " << param[0];
        return -2;
    }
    
    auto& fi = it->second;

    if (param.size() - 1 < fi.min_params)
    {
        LogError() << "GM param num not enough: " << param[0];
        return -3;
    }

    auto& f = fi.func;
    if (out_str == nullptr)
    {
        std::string _;
        return f(peer_id, param, _);
    }

    return f(peer_id, param, *out_str);
}