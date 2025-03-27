#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <stdexcept>
#include <cstdlib>
#include <limits>
#include <cctype>
#include <sstream>
#include <type_traits>
namespace Utils
{

    // 辅助函数：去除字符串前后空白字符
    std::string trim(const std::string &str)
    {
        auto front = std::find_if_not(str.begin(), str.end(),
                                      [](int c)
                                      { return std::isspace(c); });
        auto back = std::find_if_not(str.rbegin(), str.rend(),
                                     [](int c)
                                     { return std::isspace(c); })
                        .base();
        return (back <= front) ? std::string() : std::string(front, back);
    }

    // 布尔类型特化
    template <typename T>
    typename std::enable_if<std::is_same<T, bool>::value, T>::type
    strto(const std::string &str)
    {
        std::string s = trim(str);
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);

        if (s == "true" || s == "1")
            return true;
        if (s == "false" || s == "0")
            return false;

        throw std::invalid_argument("Invalid boolean: " + str);
    }

    // 整数类型转换（支持有符号/无符号）
    template <typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, T>::type
    strto(const std::string &str)
    {
        std::string s = trim(str);
        if (s.empty())
            throw std::invalid_argument("Empty string");

        char *end;
        long long value;
        unsigned long long uvalue;
        const bool is_signed = std::is_signed<T>::value;

        errno = 0;
        if (is_signed)
        {
            value = std::strtoll(s.c_str(), &end, 10);
        }
        else
        {
            uvalue = std::strtoull(s.c_str(), &end, 10);
        }

        if (end != s.c_str() + s.length())
            throw std::invalid_argument("Invalid characters: " + str);

        if (errno == ERANGE)
        {
            throw std::out_of_range("Out of range: " + str);
        }

        if (is_signed)
        {
            if (value < static_cast<long long>(std::numeric_limits<T>::min()) ||
                value > static_cast<long long>(std::numeric_limits<T>::max()))
            {
                throw std::out_of_range("Value exceeds type limits: " + str);
            }
            return static_cast<T>(value);
        }
        else
        {
            if (uvalue > static_cast<unsigned long long>(std::numeric_limits<T>::max()))
            {
                throw std::out_of_range("Value exceeds type limits: " + str);
            }
            return static_cast<T>(uvalue);
        }
    }

    // 浮点类型转换
    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, T>::type
    strto(const std::string &str)
    {
        std::string s = trim(str);
        if (s.empty())
            throw std::invalid_argument("Empty string");

        char *end;
        double value = std::strtod(s.c_str(), &end);

        if (end != s.c_str() + s.length())
            throw std::invalid_argument("Invalid characters: " + str);

        if (errno == ERANGE)
        {
            throw std::out_of_range("Out of range: " + str);
        }

        if (value < -std::numeric_limits<T>::max())
            return -std::numeric_limits<T>::infinity();

        if (value > std::numeric_limits<T>::max())
            return std::numeric_limits<T>::infinity();

        return static_cast<T>(value);
    }

    // 流式转换（支持自定义类型）
    template <typename T>
    typename std::enable_if<!std::is_arithmetic<T>::value, T>::type
    strto(const std::string &str)
    {
        std::string s = trim(str);
        std::stringstream ss(s);
        T value;
        if (!(ss >> value) || !ss.eof())
        {
            throw std::runtime_error("Conversion failed: " + str);
        }
        return value;
    }

}

#endif // UTILS_H