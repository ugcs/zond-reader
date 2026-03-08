#include <string_utils.h>
#include <exception.h>

#include <cstring>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <iomanip>


std::string
StringFormat(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    std::string result = StringFormatV(strlen(msg) * 2, msg, args);
    va_end(args);
    return result;
}

std::string
StringFormat(size_t preallocSize, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    std::string result = StringFormatV(preallocSize, msg, args);
    va_end(args);
    return result;
}

std::string
StringFormatV(const char *msg, va_list args)
{
    return StringFormat(strlen(msg) * 2, msg, args);
}

std::string
StringFormatV(size_t preallocSize, const char *msg, va_list args)
{
    va_list _args;
    va_copy(_args, args);
    std::string result(preallocSize, '\0');
    ssize_t n = vsnprintf(&result.front(), preallocSize, msg, _args);
    va_end(_args);
    if (n < 0) {
        return "<FormatError>";
    }
    if (static_cast<size_t>(n) < preallocSize) {
        result.resize(n);
        return result;
    }
    preallocSize = n + 1;
    result.resize(preallocSize, '\0');
    va_copy(_args, args);
    n = vsnprintf(&result.front(), preallocSize, msg, _args);
    va_end(_args);
    if (n < 0 || static_cast<size_t>(n) >= preallocSize) {
        return "<FormatError>";
    }
    result.resize(n);
    return result;
}

std::string
StringSubstituteTemplate(const std::string &tpl,
                         const std::map<std::string, std::string> &env,
                         StsUnresolvedMode unresolvedMode)
{
    std::string result;
    ssize_t nameOffset = -1;
    bool bsSeen = false;

    auto IsWordChar = [](char c){
        return (c >= '0' && c <= '9') ||
               (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               c == '_' || c == '-';
    };

    size_t tplSize = tpl.size();
    for (size_t idx = 0; idx < tplSize; idx++) {
        char c = tpl[idx];

        if (nameOffset != -1) {
            if (idx == static_cast<size_t>(nameOffset + 1)) {
                if (c != '{') {
                    result += '$';
                    result += c;
                    nameOffset = -1;
                }
                continue;
            }
            if (c == '}') {
                auto it = env.find(tpl.substr(nameOffset + 2, idx - nameOffset - 2));
                if (it == env.end()) {
                    switch (unresolvedMode) {
                    case StsUnresolvedMode::INSERT_REF:
                        result += tpl.substr(nameOffset, idx - nameOffset + 1);
                        break;
                    case StsUnresolvedMode::INSERT_EMPTY:
                        break;
                    case StsUnresolvedMode::THROW_EXCEPTION:
                        THROW(InternalErrorException, "Unresolved variable reference: " <<
                              tpl.substr(nameOffset + 2, idx - nameOffset - 2));
                    }
                    nameOffset = -1;
                    continue;
                }
                result += it->second;
                nameOffset = -1;
                continue;
            }
            if (!IsWordChar(c)) {
                result += tpl.substr(nameOffset, idx - nameOffset + 1);
                nameOffset = -1;
            }

        } else {
            if (c == '\\') {
                if (bsSeen) {
                    result += '\\';
                } else {
                    bsSeen = true;
                }
                continue;
            }
            if (bsSeen) {
                bsSeen = false;
                if (c == '$' || c == '\\') {
                    result += c;
                    continue;
                }
                result += '\\';
                result += c;
                continue;
            }
            if (c == '$') {
                nameOffset = idx;
                continue;
            }
            result += c;
        }
    }

    if (nameOffset != -1) {
        result += tpl.substr(nameOffset);
    }

    return result;
}

namespace {

template <typename TSep>
size_t
StringSplitSepLen(const TSep &sep);

template <>
constexpr size_t
StringSplitSepLen<char>(const char &)
{
    return 1;
}

template <>
size_t
StringSplitSepLen<std::string>(const std::string &sep)
{
    return sep.size();
}

template <typename TSep>
std::vector<std::string>
StringSplitImpl(const std::string &s, const TSep &sep)
{
    std::vector<std::string> result;
    size_t sepLen = StringSplitSepLen(sep);
    size_t pos = 0;
    while (true) {
        size_t sepPos = s.find(sep, pos);
        if (sepPos == std::string::npos) {
            break;
        }
        if (sepPos != pos) {
            result.emplace_back(s.substr(pos, sepPos - pos));
        }
        pos = sepPos + sepLen;
    }
    if (pos < s.size()) {
        result.emplace_back(s.substr(pos));
    }
    return result;
}

} /* anonymous namespace */

std::vector<std::string>
StringSplit(const std::string &s, char sep)
{
    return StringSplitImpl(s, sep);
}

std::vector<std::string>
StringSplit(const std::string &s, const std::string &sep)
{
    return StringSplitImpl(s, sep);
}

std::string
ToFixedString(double d, int precision)
{
    std::string buf;
    buf.resize(32);

    //XXX use std::to_chars() when compiler updated (tested with GCC 11).
    // auto result = std::to_chars(buf.data(), buf.data() + buf.size(), d, std::chars_format::fixed,
    //                             precision);
    // if (result.ec != std::errc()) {
    //     throw std::runtime_error("Failed to convert double to string");
    // }
    // buf.resize(result.ptr - buf.data());

    int sz = snprintf(&buf[0], buf.size(), "%.*f", precision, d);
    if (static_cast<size_t>(sz) < buf.size()) {
        buf.resize(sz);
    }
    return buf;
}

bool
StringStartsWith(std::string_view s, std::string_view prefix)
{
    if (s.size() < prefix.size()) {
        return false;
    }
    return s.substr(0, prefix.size()) == prefix;
}

std::string
ToHex(const void *data, size_t size)
{
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    for (const uint8_t *p = reinterpret_cast<const uint8_t *>(data);
         p < reinterpret_cast<const uint8_t *>(data) + size;
         p++) {
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(*p);
    }
    return ss.str();
}
