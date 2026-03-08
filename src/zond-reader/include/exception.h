#ifndef INCLUDE_EXCEPTION_H
#define INCLUDE_EXCEPTION_H

#include <stdarg.h>
#include <defs.h>
#include <sstream>
#include <vector>


/** Base class for all exceptions in this component. */
class Exception: public std::exception {
public:
    /** Indentation size in number of spaces. Used for better messages formatting. */
    static constexpr int INDENT_SIZE = 4;

    Exception(const char *className, const char *file, int line, const std::string &msg);

    /** Returns full message (including source location and cause chain). */
    virtual const char*
    what() const noexcept override;

    /** Get message as it was specified on exception construction. */
    std::string
    GetMessage() const
    {
        return msg;
    }

    /** Extended message may include additional information, e.g. some exception parameters. */
    virtual std::string
    GetExtendedMessage() const
    {
        return msg;
    }

    /** @return True if the exception should be considered fatal and cause panic in debug build. */
    virtual bool
    IsFatal() const
    {
        return false;
    }

    std::string
    GetFullMessage(int indent = 0) const;

    std::string
    CreateFullMessage(int indent = 0) const;

    const char *
    GetClassName() const
    {
        return className;
    }

    const char *
    GetSourceFileName() const
    {
        return file;
    }

    int
    GetSourceLine() const
    {
        return line;
    }

    static std::string
    GetFullMessage(std::exception_ptr e, int indent = 0);

    static std::string
    GetFullMessage(const std::exception &e, int indent = 0);

    void
    AddSuppressed(std::exception_ptr e);

    /** Crash the application for better problem analysis. Recommended to use in debug builds only.
     */
    static void
    Panic();

    /** Check if the exception is fatal and panic if it is. This is debug feature and should be used
     * in debug builds only.
     */
    static void
    CheckFatal(const std::exception &e);

    /** Get error message for last system error (platform specific). */
    static std::string
    GetSystemErrorMessage();

protected:
    const char * const className;
    const char * const file;
    const int line;
    /* Excessive allocations are not desirable in exception object but will do them for the sake of
     * development convenience. Hoping it is not out of memory exception being handled (which is not
     * recoverable anyway).
     */
    const std::string msg;
    /** Full message is constructed here on the first request. Need to store it in the exception
     * instance because of what() method signature.
     */
    mutable std::string fullMsg;
    mutable int fullMsgIndent = 0;
    /** List of suppressed exceptions if any. */
    std::vector<std::exception_ptr> suppressed;
};

#ifdef SDK_FATAL_EXCEPTIONS
#   define EXCEPTION_CHECK_FATAL(__e) Exception::CheckFatal(__e)
#else
#   define EXCEPTION_CHECK_FATAL(__e)
#endif

#define MAKE_EXCEPTION(__excName, __msg, ...) \
    ({ \
        std::stringstream __ss; \
        __ss << __msg; \
        auto __e = __excName(STR(__excName), __FILE__, __LINE__, __ss.str(), ## __VA_ARGS__); \
        EXCEPTION_CHECK_FATAL(__e); \
        __e; \
    })

#define MAKE_EXCEPTION_PTR(__excName, __msg, ...) \
    std::make_exception_ptr(MAKE_EXCEPTION(__excName, __msg, ## __VA_ARGS__))

#define MAKE_EXCEPTION_NO_MSG(__excName, ...) \
    __excName(STR(__excName), __FILE__, __LINE__, ## __VA_ARGS__)

#define MAKE_EXCEPTION_NO_MSG_PTR(__excName, ...) \
    std::make_exception_ptr(MAKE_EXCEPTION_NO_MSG(__excName, ## __VA_ARGS__))

#define THROW(__excName, __msg, ...) \
    throw MAKE_EXCEPTION(__excName, __msg, ## __VA_ARGS__)

#define THROW_NO_MSG(__excName, ...) \
    throw MAKE_EXCEPTION_NO_MSG(__excName, ## __VA_ARGS__)

#define THROW_NESTED(__excName, __msg, ...) \
    std::throw_with_nested(MAKE_EXCEPTION(__excName, __msg, ## __VA_ARGS__))

#define THROW_NESTED_NO_MSG(__excName, __msg, ...) \
    std::throw_with_nested(MAKE_EXCEPTION_NO_MSG(__excName, ## __VA_ARGS__))

#define DEFINE_EXCEPTION(__clsName) \
    class __clsName: public Exception { \
    public: \
        using Exception::Exception; \
    };

/** Exception to throw when debugging assertion fires. */
DEFINE_EXCEPTION(DebugAssertException);
/** Indicates that some invalid parameter was passed to an API call. */
DEFINE_EXCEPTION(InvalidParamException);
/** Indicates that the operation is invalid in current state. */
DEFINE_EXCEPTION(InvalidOpException);
/** Some unexpected internal error occurred. Potential software bug. */
DEFINE_EXCEPTION(InternalErrorException);
/** Exception for system call failure. */
DEFINE_EXCEPTION(SystemException);
/** Not enough system resources for requested operation. */
DEFINE_EXCEPTION(InsufficientResourcesException);
/** Feature not implemented. */
DEFINE_EXCEPTION(NotImplementedException);

#define TODO(__feature_name) \
    THROW(NotImplementedException, "Feature not implemented: " << (__feature_name))

#endif /* INCLUDE_EXCEPTION_H */
