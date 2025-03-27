#pragma once

#include <unordered_map>
#include <vector>

#include "noncopyable.h"
#include "Timestamp.h"
// #include "Channel.h"
// #include "EventLoop.h"

namespace dwt {


class Channel;
class EventLoop;

// IO多路复用基类
class Poller : noncopyable {

public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);

    virtual ~Poller() = default; // 虚析构

    virtual Timestamp poll(int timeOutMs, ChannelList* activeChannels) = 0; // 纯虚

    virtual void updateChannel(Channel* channel) = 0;

    virtual void removeChannel(Channel* channel) = 0;

    // 判断channel是否在poller中
    virtual bool hasChannel(Channel* channel) const;

    // eventloop通过该接口获取IO多路复用的具体实现
    static Poller* newDefaultPoller(EventLoop* loop);


protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    
    ChannelMap m_channels;

private:

    EventLoop* m_ownerLoop; // 所属的事件循环

};

}