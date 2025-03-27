#pragma once

#include <iostream>
#include <string>

namespace dwt {


class Timestamp {

public:
    Timestamp();

    explicit Timestamp(int64_t microSec);

    static Timestamp now();

    std::string toString() const;

private:
    int64_t m_sec;

};

}