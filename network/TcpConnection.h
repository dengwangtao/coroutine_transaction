#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"

#include <memory>
#include <string>
#include <atomic>

namespace dwt{


class EventLoop;
class Channel;
class Socket;


class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {

public:
    TcpConnection(
        EventLoop* loop, const std::string& name, int sockfd,
        const InetAddress& localAddr, const InetAddress& peerAddr);
    
    ~TcpConnection();

    EventLoop* getLoop() const { return m_loop; }
    const std::string& name() const { return m_name; }
    const InetAddress& localAddress() const { return m_localAddr; }
    const InetAddress& peerAddress() const { return m_peerAddr; }
    bool connected() const { return m_state == StateE::kConnected; }

    void send(const void* message, size_t len);
    void send(const std::string& str);

    void shutdown();


    void setConnectionCallback(const ConnectionCallback& cb) {
        m_connectionCallback = cb;
    }

    void setMessageCallback(const MessageCallback& cb) {
        m_messageCallback = cb;
    }

    void setCloseCallback(const CloseCallback& cb) {
        m_closeCallback = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        m_writeCompleteCallback = cb;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWatermark) {
        m_highWaterMarkCallback = cb;
        m_highWaterMark = highWatermark;
    }

    void connectEstablished();  // only call once

    void connectDestoryed();    // only call once

private:
    enum StateE {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

    void handleRead(Timestamp receiceTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();

    void setState(StateE s) { m_state = s; }


    EventLoop* m_loop;
    const std::string m_name;
    std::atomic<StateE> m_state; // StateE 类型
    bool m_reading;

    std::unique_ptr<Socket> m_socket;
    std::unique_ptr<Channel> m_channel;

    const InetAddress m_localAddr;
    const InetAddress m_peerAddr;

    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    CloseCallback m_closeCallback;
    WriteCompleteCallback m_writeCompleteCallback;

    HighWaterMarkCallback m_highWaterMarkCallback;

    size_t m_highWaterMark;

    Buffer m_inputBuffer;
    Buffer m_outputBuffer;
};



}