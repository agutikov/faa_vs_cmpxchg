#pragma once

#include <string>
#include <cstdio>
#include <sstream>
#include <cstdarg>
#include <thread>

template<typename T>
std::string to_string(const T& v)
{
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

void tprintf(const char* fmt, ...)
{
    auto tid = to_string(std::this_thread::get_id());
    printf("%s ", tid.c_str());

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
