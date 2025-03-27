#pragma once

#include <vector>
#include <stddef.h>
#include <string>

namespace dwt {



/**
 * A buffer class modeled after org.jboss.netty.buffer.channelBuffer
 * @code
 * +--------------------+---------------------------+---------------------------+
 * | prependable bytes  |      readable bytes       |       writeable bytes     |
 * |                    |        (CONTENT)          |                           |
 * +--------------------+---------------------------+---------------------------+
 * |                    |                           |                           |
 * 0       <=      readerIndex      <=         writeIndex         <=           size
 * @endcode
 */
class Buffer {

public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : m_buffer(kCheapPrepend + kInitialSize)
        , m_readerIdx(kCheapPrepend)
        , m_writerIdx(kCheapPrepend) {

        // 初始化m_buffer的大小
    }


    size_t readableBytes() const {
        return m_writerIdx - m_readerIdx;
    }

    size_t writableBytes() const {
        return m_buffer.size() - m_writerIdx;
    }

    size_t prependableBytes() const {
        return m_readerIdx;
    }

    // 返回可读数据的起始地址
    const char* peek() const {
        return begin() + m_readerIdx;
    }

    void retrieve(size_t len) {
        if(len < readableBytes()) {
            m_readerIdx += len;
        } else {
            retrieveAll();
        }
    }

    void retrieveAll() {
        m_readerIdx = m_writerIdx = kCheapPrepend;
    }

    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len) {
        std::string res(peek(), len);
        retrieve(len);
        return res;
    }

    void ensureWritableBytes(size_t len) {
        if(writableBytes() < len) {
            makeSpace(len);
        }
    }

    void append(const std::string& str) {
        append(str.data(), str.size());
    }

    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len); // m_writerIdx += len;
    }




    char* beginWrite() {
        return begin() + m_writerIdx;
    }

    const char* beginWrite() const {
        return begin() + m_writerIdx;
    }

    void hasWritten(size_t len) {
        m_writerIdx += len;
    }


    // 从fd上读数据
    size_t readFd(int fd, int* saveErrno);
    size_t writeFd(int fd, int* saveErrno);

private:

    char* begin() {
        return &m_buffer[0];
    }

    const char* begin() const {
        return &m_buffer[0];
    }

    void makeSpace(size_t len) {
        if(writableBytes() + prependableBytes() < len + kCheapPrepend) {
            m_buffer.resize(m_writerIdx + len);
        } else {
            size_t readable = readableBytes();

            std::copy(m_buffer.begin() + m_readerIdx, m_buffer.begin() + m_readerIdx + readable, m_buffer.begin() + kCheapPrepend);

            m_readerIdx = kCheapPrepend;
            m_writerIdx = m_readerIdx + readable;
        }
    }


    std::vector<char> m_buffer;
    size_t m_readerIdx;
    size_t m_writerIdx;

};


}