#include <exception.h>
#include <string_utils.h>
#include <cstring>


namespace {

const char *
GetFileBaseName(const char *file)
{
    size_t i;
    ssize_t lastSlash = -1;
    for (i = 0; file[i]; i++) {
        if (file[i] == '/'
#           ifdef _WIN32
            || file[i] == '\\'
#           endif
            ) {
            lastSlash = i;
        }
    }
    if (lastSlash != -1) {
        return file + lastSlash + 1;
    }
    return file;
}

} /* anonymous namespace */


Exception::Exception(const char *className, const char *file, int line, const std::string &msg):
    className(className), file(GetFileBaseName(file)), line(line), msg(msg)
{}

const char*
Exception::what() const noexcept
{
    if (!fullMsg.empty()) {
        return fullMsg.c_str();
    }
    fullMsg = CreateFullMessage();
    return fullMsg.c_str();
}

std::string
Exception::GetFullMessage(int indent) const
{
    if (!fullMsg.empty() && indent == 0) {
        return fullMsg;
    }
    return CreateFullMessage(indent);
}

std::string
Exception::CreateFullMessage(int indent) const
{
    std::string result;
    result.append(indent * INDENT_SIZE, ' ');
    result += className;
    result += " at [";
    result += file;
    result += ':';
    result += std::to_string(line);
    result += "]: ";
    result += GetExtendedMessage();

    if (!suppressed.empty()) {
        result += "\n";
        result.append(indent * INDENT_SIZE, ' ');
        result += "Suppressed:\n";
        bool isFirst = true;
        for (std::exception_ptr e: suppressed) {
            if (!isFirst) {
                result += '\n';
            } else {
                isFirst = false;
            }
            result += GetFullMessage(e, indent + 1);
        }
    }

    try {
        std::rethrow_if_nested(*this);
    } catch (Exception &e) {
        result += "\n";
        result.append(indent * INDENT_SIZE, ' ');
        result += "Caused by:\n";
        result += e.CreateFullMessage(indent);
    } catch (std::exception &e) {
        result += "\n";
        result.append(indent * INDENT_SIZE, ' ');
        result += "Caused by:\n";
        result += GetFullMessage(e, indent);
    } catch(...) {
        result += "\n";
        result.append(indent * INDENT_SIZE, ' ');
        result += "Caused by unknown exception type";
    }
    return result;
}

void
Exception::AddSuppressed(std::exception_ptr e)
{
    suppressed.emplace_back(e);
}

std::string
Exception::GetFullMessage(std::exception_ptr ptr, int indent)
{
    if (!ptr) {
        return std::string(indent * INDENT_SIZE, ' ') + "None";
    }
    try {
        std::rethrow_exception(ptr);
    } catch (Exception &e) {
        return e.GetFullMessage(indent);
    } catch (std::exception &e) {
        return GetFullMessage(e, indent);
    } catch (...) {
        return std::string(indent * INDENT_SIZE, ' ') + "Unknown exception type";
    }
}

std::string
Exception::GetFullMessage(const std::exception &e, int indent)
{
    if (const Exception *ePtr = dynamic_cast<const Exception *>(&e)) {
        return ePtr->CreateFullMessage(indent);
    }
    std::string result;
    if (const std::system_error *ePtr = dynamic_cast<const std::system_error *>(&e)) {
        result.append(indent * INDENT_SIZE, ' ');
        result += ePtr->what();
        result += " (Code ";
        result += std::to_string(ePtr->code().value());
        result += ")";
    } else {
        result.append(indent * INDENT_SIZE, ' ');
        result += e.what();
    }

    try {
        std::rethrow_if_nested(e);
    } catch (Exception &eNested) {
        result += "\n";
        result.append(indent * INDENT_SIZE, ' ');
        result += "Caused by:\n";
        result += eNested.CreateFullMessage(indent);
    } catch (std::exception &eNested) {
        result += "\n";
        result.append(indent * INDENT_SIZE, ' ');
        result += "Caused by:\n";
        result += GetFullMessage(eNested, indent);
    } catch(...) {
        result += "\n";
        result.append(indent * INDENT_SIZE, ' ');
        result += "Caused by unknown exception type";
    }
    return result;
}

void
Exception::Panic()
{
    *reinterpret_cast<volatile int *>(0) = 42;
}

void
Exception::CheckFatal(const std::exception &e)
{
    const Exception *_e;
    if (typeid(e) == typeid(DebugAssertException) ||
        typeid(e) == typeid(InvalidParamException) ||
        typeid(e) == typeid(InvalidOpException) ||
        typeid(e) == typeid(InternalErrorException) ||
        typeid(e) == typeid(NotImplementedException) ||
        ((_e = dynamic_cast<const Exception *>(&e)) && _e->IsFatal())) {

        Panic();
    }
}

#ifdef _WIN32
//XXX not implemented
#else
std::string
Exception::GetSystemErrorMessage()
{
    char buf[2048], *pBuf;
    int code = errno;
#   if ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE) || \
        (defined(ANDROID) && (!defined(__USE_GNU) || __ANDROID_API__ < 23)) || \
        defined(__APPLE__) || defined(__WEB__)
        /* POSIX version. */
        if (strerror_r(code, buf, sizeof(buf))) {
            return StringFormat("errno = %d (failed to get message)", code);
        }
        pBuf = buf;
#   else
        /* GNU version. */
        errno = 0;
        pBuf = strerror_r(code, buf, sizeof(buf));
        if (errno != 0) {
            return StringFormat("errno = %d (failed to get message)", code);
        }
#   endif
    return StringFormat("System error [%d]: %s", code, pBuf);
}
#endif /* platform. */
