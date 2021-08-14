#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>

#ifndef NDEBUG

#define TODO() \
do \
{ \
    std::cerr << "[<<< TODO >>>]: " __FILE__ ":" << __LINE__ << "\n"; \
} \
while (0)

#else

#define TODO() void(0)

#endif

template <typename T>
[[nodiscard]] inline std::string intToHexStr(T value)
{
    std::stringstream ss;
    ss << std::hex << value;
    return ss.str();
}

[[nodiscard]] inline std::string strToLower(std::string str)
{
    for (size_t i{}; i < str.size(); ++i)
        str[i] = std::tolower(str[i]);
    return str;
}
