#include "Buffer.h"

#include <sys/uio.h>
#include <unistd.h>

namespace dwt {

size_t Buffer::readFd(int fd, int* saveErrno) {

    char extrabuf[65536] = {0}; // 64kB

    struct iovec vec[2];

    const size_t writable = writableBytes();

    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    // 保证一次至少读出64k的数据
    const int iovecCount = (writable < sizeof extrabuf) ? 2 : 1;

    // 分散读
    const size_t n = ::readv(fd, vec, iovecCount);

    if(n < 0) {
        *saveErrno = errno;
    } else if(static_cast<size_t>(n) < writable) {  // 只写入到了 m_buffer 中
        hasWritten(n);
    } else {
        m_writerIdx = m_buffer.size();
        append(extrabuf, n - writable); // 写入到 extrabuf 的数据长度为 n - writable
    }

    return n;
}

// note: fd is nonblocking
size_t Buffer::writeFd(int fd, int* saveErrno) {
    size_t n = ::write(fd, peek(), readableBytes());
    if(n < 0) {
        *saveErrno = errno;
    }
    return n;
}


}