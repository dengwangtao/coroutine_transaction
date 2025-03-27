#pragma once

namespace dwt {



/**
 * noncopyable作为基类被继承
 * 派生类不能被复制
*/

class noncopyable {

public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;

private:

};


}