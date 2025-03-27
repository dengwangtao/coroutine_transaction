#include "EpollPoller.h"

#include "Logger.h"
#include "Channel.h"

#include <unistd.h>
#include <string.h>

namespace dwt {

const int kNew = -1;            // channel->index初始化为-1
const int kAdded = 1;
const int kDeleted = 2;


EpollPoller::EpollPoller(EventLoop* loop)
    : Poller(loop)
    , m_epfd(::epoll_create1(EPOLL_CLOEXEC))
    , m_events(kInitEventListSize) {
    
    // 构造
    if(m_epfd < 0) {
        LOG_FATAL("epoll_create1 exec error!(errno=%d) file: %s, line: %d", errno, __FILE__, __LINE__);
    }
}

EpollPoller::~EpollPoller() {
    ::close(m_epfd);
}


Timestamp EpollPoller::poll(int timeOutMs, ChannelList* activeChannels) {

    LOG_DEBUG("function=EpollPoller::poll | fd total count=%d", m_channels.size());

    int numEvents = ::epoll_wait(m_epfd, &m_events[0], static_cast<int>(m_events.size()), timeOutMs);
    int saveErrno = errno;
    Timestamp now = Timestamp::now();

    if(numEvents > 0) {

        LOG_DEBUG("function=EpollPoller::poll | %d event happend", numEvents);
        fillActiveChannels(numEvents, activeChannels);

        if(static_cast<size_t>(numEvents) == m_events.size()) {
            m_events.resize(m_events.size() * 2);
        }

    } else if(numEvents == 0) { // epoll_wait超时返回

        LOG_DEBUG("function=EpollPoller::poll | no event happend", numEvents);

    } else {

        if(saveErrno != EINTR) { // 不是被系统调用打断
            errno = saveErrno;
            LOG_ERROR("EpollPoller::poll() error");
        }

    }
    return now;
}

void EpollPoller::updateChannel(Channel* channel) {
    const int index = channel->index();
    int fd = channel->fd();

    LOG_INFO("function=EpollPoller::updateChannel | fd=%d index=%d" , fd, channel->index());

    if(index == kNew || index == kDeleted) {

        if(index == kNew) {

            m_channels[fd] = channel;
            
        } else {    // index == kDeleted

        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);

    } else {        // Added
        if(channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel) {
    int fd = channel->fd();
    int index = channel->index();

    LOG_INFO("function=EpollPoller::removeChannel | fd=%d" , fd);

    size_t n = m_channels.erase(fd);

    if(index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}


void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    for(int i = 0; i < numEvents; ++ i) {
        Channel* channel = static_cast<Channel*>(m_events[i].data.ptr);
        channel->set_revents(m_events[i].events);       // 哪些事件被触发了
        activeChannels->push_back(channel);
    }
}

void EpollPoller::update(int operation, Channel* channel) {

    int fd = channel->fd();
    int events = channel->events();

    struct epoll_event ev;
    ::memset(&ev, 0, sizeof(ev));
    ev.events = events;
    ev.data.ptr = channel;

    if(::epoll_ctl(m_epfd, operation, fd, &ev) < 0) {
        
        if(operation == EPOLL_CTL_DEL) {
            // 错误
            LOG_ERROR("epoll_ctl EPOLL_CTL_DEL error");
        } else {
            // 致命错误
            LOG_FATAL("epoll_ctl EPOLL_CTL_ADD/MOD fatal error");
        }

    }
}



}   // end namespace dwt