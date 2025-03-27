#pragma once

#include "Poller.h"
#include <vector>
#include <sys/epoll.h>

namespace dwt {

class EpollPoller : public Poller {

public:

    EpollPoller(EventLoop* loop);

    ~EpollPoller() override;

    Timestamp poll(int timeOutMs, ChannelList* activeChannels) override;

    void updateChannel(Channel* channel) override;

    void removeChannel(Channel* channel) override;


    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

    void update(int operation, Channel* channel);


private:
    static const int kInitEventListSize = 16;

    using EventList = std::vector<epoll_event>;

    int m_epfd;
    EventList m_events;

    // ChannelMap m_channels; 继承自父类

};


}