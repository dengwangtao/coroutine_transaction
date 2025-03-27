#include "InetAddress.h"
#include <string.h>

namespace dwt{


InetAddress::InetAddress() {
    memset(&m_addr, 0, sizeof(m_addr));
}

InetAddress::InetAddress(const std::string& IP, uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    inet_pton(AF_INET, IP.c_str(), &m_addr.sin_addr.s_addr);
    // inet_addr() 方法
}
InetAddress::InetAddress(const struct sockaddr_in& addr): m_addr(addr) {

}

std::string InetAddress::toIp() const {
    char buf[64] = {0};
    inet_ntop(AF_INET, &m_addr.sin_addr.s_addr, buf, sizeof(buf));
    return buf;
}
uint16_t InetAddress::toPort() const {
    return ntohs(m_addr.sin_port);
}
std::string InetAddress::toIpPort() const {
    char buf[64] = {0};
    inet_ntop(AF_INET, &m_addr.sin_addr.s_addr, buf, sizeof(buf));
    sprintf(buf + strlen(buf), ":%d", ntohs(m_addr.sin_port));
    return buf;
}

const struct sockaddr_in* InetAddress::getSockAddr() const {
    return &m_addr;
}

}