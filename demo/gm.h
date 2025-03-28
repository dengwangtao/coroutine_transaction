#pragma once
#ifndef DEMO_GM_H
#define DEMO_GM_H

#include "common_def.h"
#include "singleton.h"
#include <unordered_map>
#include <functional>
#include <vector>

using GMParam = std::vector<std::string>;
using GMFunc = std::function<s32(u64 peer_id, const GMParam&, std::string& out_str)>;
struct GMFuncInfo
{
    u32 min_params = 1;
    std::string desc;
    GMFunc func;
    GMFuncInfo() = default;
    GMFuncInfo(u32 min_params, const std::string& desc, const GMFunc& func)
        : min_params(min_params), desc(desc), func(func)
    {}
};

using GMMap = std::unordered_map<std::string, GMFuncInfo>;

class GMMgr : public Singleton<GMMgr>
{
    
public:
    s32 RegisterGM(const std::string& name, const GMFuncInfo& func_info);
    s32 CallGM(u64 peer_id, const std::string& token, std::string* out_str = nullptr);
private:
    GMMap gm_map_;
};


s32 RegisterAllGM();


#define g_gm_mgr GMMgr::Instance()

#endif // DEMO_GM_H