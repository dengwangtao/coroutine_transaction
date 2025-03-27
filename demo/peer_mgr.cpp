#include "peer_mgr.h"



u64 PeerMgr::add_peer(const std::string& peer_str)
{
    auto it = peer_map_.find(peer_str);
    if (it == peer_map_.end())
    {
        u64 peer_id = get_next_free_peer_id();
        peer_map_[peer_str] = peer_id;
        return peer_id;
    }
    else
    {
        return it->second;
    }
}
u64 PeerMgr::get_peer_id(const std::string& peer_str)
{
    auto it = peer_map_.find(peer_str);
    if (it == peer_map_.end())
    {
        return 0;
    }
    else
    {
        return it->second;
    }
}

void PeerMgr::remove_peer(const std::string& peer_str)
{
    auto it = peer_map_.find(peer_str);
    if (it!= peer_map_.end())
    {
        u64 peer_id = it->second;
        peer_map_.erase(it);
        free_peer_id_.insert(peer_id);
    }
}

u64 PeerMgr::get_next_free_peer_id()
{
    if (free_peer_id_.empty())
    {
        return next_user_id_++;
    }
    else
    {
        u64 peer_id = *free_peer_id_.begin();
        free_peer_id_.erase(free_peer_id_.begin());
        return peer_id;
    }
}