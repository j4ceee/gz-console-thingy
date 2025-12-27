#pragma once

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <unordered_map>

namespace util
{
template <typename... Args> std::string string_format(const std::string &format, Args... args)
{
    size_t                  size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1);
}

static std::wstring ToWideString(const std::string &str)
{
    wchar_t buf[0xfff] = {};
    size_t  converted;
    mbstowcs_s(&converted, buf, str.c_str(), sizeof(buf));
    return std::wstring(buf);
}
}; // namespace util
