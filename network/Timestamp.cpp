#include "Timestamp.h"

#include <sys/time.h>

namespace dwt{

Timestamp::Timestamp():m_sec(0) {

}

Timestamp::Timestamp(int64_t sec):m_sec(sec) {

}

// static
Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return Timestamp(tv.tv_sec);
}

std::string Timestamp::toString() const {
    char buf[128] = {0};
    struct tm* tm_time = localtime(&m_sec);
    snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
             tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
    return buf;
}


}