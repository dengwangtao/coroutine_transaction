#include "TcpConnection.h"

#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

namespace dwt{

static EventLoop* checkLoopNotNull(EventLoop* loop) {
    if(loop == nullptr) {
        LOG_FATAL("%s %s %d TcpConnection Loop is nullptr", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}





TcpConnection::TcpConnection(
        EventLoop* loop, const std::string& name, int sockfd,
        const InetAddress& localAddr, const InetAddress& peerAddr)

    : m_loop(checkLoopNotNull(loop))
    , m_name(name)
    , m_state(StateE::kConnecting)
    , m_reading(true)
    , m_socket(std::make_unique<Socket>(sockfd))
    , m_channel(std::make_unique<Channel>(loop, sockfd))
    , m_localAddr(localAddr)
    , m_peerAddr(peerAddr)
    , m_highWaterMark(64 * 1024 * 1024) /* 64M */ {
    
    // 构造函数
    m_channel->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1)
    );

    m_channel->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this)
    );

    m_channel->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this)
    );

    m_channel->setErrorCallback(
        std::bind(&TcpConnection::handleError, this)
    );

    LOG_INFO("TcpConnection::ctor[%s] at fd = %d", m_name.c_str(), sockfd);

    m_socket->setKeepAlive(true);
}
    
TcpConnection::~TcpConnection() {
    LOG_INFO("TcpConnection::dtor[%s] at fd = %d state = %d", m_name.c_str(), m_channel->fd(), m_state.load());
}



void TcpConnection::send(const void* message, size_t len) {
    if(m_state == StateE::kConnected) {
        if(m_loop->isInLoopThread()) {
            sendInLoop(message, len);
        } else {
            m_loop->runInLoop(
                std::bind(&TcpConnection::sendInLoop, this, message, len)
            );
        }
    }
}

void TcpConnection::send(const std::string& str) {
    send(str.data(), str.size());
}

void TcpConnection::sendInLoop(const void* message, size_t len) {

    size_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if(m_state == StateE::kDisconnected) {
        LOG_WARN("TcpConnection::sendInLoop: disconnected, giveup writing");
        return;
    }

    if(!m_channel->isWriting() && m_outputBuffer.readableBytes() == 0) {
        // channel 第一次开始写数据, 没有待发送数据

        // 尝试直接写
        nwrote = ::write(m_channel->fd(), message, len);

        if(nwrote >= 0) {
            remaining = len - nwrote;

            if(remaining == 0 && m_writeCompleteCallback) {
                m_loop->queueInLoop(
                    std::bind(m_writeCompleteCallback, this->shared_from_this())
                );
            }

        } else { // nwrote < 0

            nwrote = 0;
            if(errno != EWOULDBLOCK) { // EWOULDBLOCK 是非阻塞没数据

                LOG_ERROR("TcpConnection::sendInLoop");
                if(errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }

            }

        }
    }

    // 放入Buffer写
    if(!faultError && remaining > 0) {

        size_t oldLen = m_outputBuffer.readableBytes();

        if(oldLen + remaining >= m_highWaterMark && oldLen < m_highWaterMark && m_highWaterMarkCallback) { // 超过高水位
            m_loop->queueInLoop(
                std::bind(m_highWaterMarkCallback, this->shared_from_this(), oldLen + remaining)
            );
        }

        m_outputBuffer.append(static_cast<const char*>(message) + nwrote, remaining);

        if(!m_channel->isWriting()) {
            m_channel->enableWriting();
        }
    }

}

// 调用shutdown时, 数据可能还在发送
// TcpConnection::handleWrite中当发送完数据, 判断kDisconnecting, 会调用shutdownInLoop
void TcpConnection::shutdown() {
    if(m_state == StateE::kConnected) {
        setState(StateE::kDisconnecting);


        m_loop->runInLoop(
            std::bind(&TcpConnection::shutdownInLoop, this)
        );
    }
}

void TcpConnection::shutdownInLoop() {
    if(!m_channel->isWriting()) {
        m_socket->shutdownWrite();  // 会触发EPOLLHUP, 调用close回调
    }
}


void TcpConnection::connectEstablished() {
    setState(StateE::kConnected);
    m_channel->tie(shared_from_this()); // channel 的 weekptr 监视当前的 TcpConnection
    m_channel->enableReading();

    m_connectionCallback(shared_from_this());
}

void TcpConnection::connectDestoryed() {
    if(m_state == StateE::kDisconnected) {
        setState(StateE::kDisconnected);

        m_channel->disableAll();
        m_connectionCallback(shared_from_this());
    }
    m_channel->remove();    
}




void TcpConnection::handleRead(Timestamp receiceTime) {
    int saveErrno = 0;
    size_t n = m_inputBuffer.readFd(m_channel->fd(), &saveErrno);

    // LOG_INFO("%s:%d TcpConnection::handleRead fd=%d getData=%lu bytes", __FILE__, __LINE__, m_channel->fd(), n);

    if(n > 0) {
        // 已连接的用户有数据到达
        m_messageCallback(this->shared_from_this(), &m_inputBuffer, receiceTime);
    } else if(n == 0) {

        handleClose();

    } else {

        errno = saveErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();

    }

}

// epoll上报, 标识当前可写,调用该回调继续写
void TcpConnection::handleWrite() {

    if(m_channel->isWriting()) {

        int saveErrno = 0;

        size_t n = m_outputBuffer.writeFd(m_channel->fd(), &saveErrno);

        if(n > 0) {

            m_outputBuffer.retrieve(n);

            if(m_outputBuffer.readableBytes() == 0) { // 全部发送完成
                m_channel->disableWriting();

                if(m_writeCompleteCallback) {

                    // 放入函数, 并唤醒线程
                    m_loop->queueInLoop(
                        std::bind(m_writeCompleteCallback, this->shared_from_this())
                    );

                }

                if(m_state == StateE::kDisconnecting) {
                    shutdownInLoop();
                }

            }

        } else {

            LOG_ERROR("TcpConnection::handleWrite");
            // handleError();
        }

    } else { // not m_channel->isWriting()

        LOG_ERROR("TcpConnection fd = %d is down, no more writing", m_channel->fd());
    }
}

void TcpConnection::handleClose() {
    LOG_INFO("TcpConnection::handleClose fd = %d, state = %d", m_channel->fd(), m_state.load());
    setState(StateE::kDisconnected);

    m_channel->disableAll();

    TcpConnectionPtr guardThis(this->shared_from_this());
    m_connectionCallback(guardThis);
    m_closeCallback(guardThis);
}

void TcpConnection::handleError() {

    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    int err = 0;
    if(::getsockopt(m_channel->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    } else {
        err = optval;
    }

    LOG_ERROR("TcpConnection::handleError name=%s SO_ERROR=%d",m_name.c_str(), err);

}





}