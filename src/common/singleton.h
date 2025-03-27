#pragma once

template<class T>
class Singleton
{
public:
    Singleton(Singleton const&) = delete;
    Singleton& operator=(Singleton const&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    static T& Instance()
    {
        static T inst;
        return inst;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
};
