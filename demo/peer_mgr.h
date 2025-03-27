#pragma once
#ifndef DEMO_PEER_MGR_H
#define DEMO_PEER_MGR_H

#include "singleton.h"
#include <unordered_map>
#include <string>
#include "common_def.h"

class PeerMgr : public Singleton<PeerMgr>
{
public:
    u64 add_peer(const std::string& peer_str);
    u64 get_peer_id(const std::string& peer_str);
    void remove_peer(const std::string& peer_str);
private:
    u64 get_next_free_peer_id();
private:
    std::unordered_map<std::string, int> peer_map_;
    std::unordered_set<u64> free_peer_id_;

    u64 next_user_id_ = 100000;
};

#define g_peer_mgr PeerMgr::Instance()

#endif // DEMO_PEER_MGR_H