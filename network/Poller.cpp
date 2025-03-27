#include "Poller.h"

#include "Channel.h"

namespace dwt {


Poller::Poller(EventLoop* loop): m_ownerLoop(loop) {

}


bool Poller::hasChannel(Channel* channel) const {
    auto item = m_channels.find(channel->fd());
    return item != m_channels.end() && item->second == channel;
}

// TODO: 放在单独文件中实现
// Poller* Poller::newDefaultPoller(EventLoop* loop);



}