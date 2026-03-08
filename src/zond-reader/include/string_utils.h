#ifndef INCLUDE_STRING_UTILS_H
#define INCLUDE_STRING_UTILS_H

#include <defs.h>
#include <string>
#include <map>
#include <vector>
#include <cstdint>


/** Returns string as by sprintf(). */
std::string
StringFormat(const char *msg, ...) FORMAT_PRINTF(1, 2);

/** @param preallocSize Hint for buffer pre-allocation size. */
std::string
StringFormat(size_t preallocSize, const char *msg, ...) FORMAT_PRINTF(2, 3);

std::string
StringFormatV(const char *msg, va_list args) FORMAT_PRINTF(1, 0);

std::string
StringFormatV(size_t preallocSize, const char *msg, va_list args) FORMAT_PRINTF(2, 0);

/** Defines how to handle unresolved variable references in StringSubstituteTemplate() function. */
enum class StsUnresolvedMode {
    /** Insert the original reference from the template. */
    INSERT_REF,
    /** Assume the variable value is empty string. */
    INSERT_EMPTY,
    /** Throw an exception. */
    THROW_EXCEPTION
};

/** Substitutes variables in the specified template to their values. Variable is specified as
 * ${var_name}. Substitution can be prevented by escaping $ by backslash. Backslash itself should
 * be escaped. Variable name should consist from word-allowed characters. In case the variable is
 * not found in the specified environment dictionary the behavior is defined via unresolvedMode
 * parameter.
 * @param tpl Template text.
 * @param env Variable names mapped to values.
 * @param unresolvedMode Defines behavior for cases when the referenced variable is not found in
 *      the provided environment.
 * @return Rendered template.
 */
std::string
StringSubstituteTemplate(const std::string &tpl,
                         const std::map<std::string, std::string> &env,
                         StsUnresolvedMode unresolvedMode = StsUnresolvedMode::INSERT_REF);

/** Split string by the specified separator. Empty components are discarded. */
std::vector<std::string>
StringSplit(const std::string &s, char sep);

std::vector<std::string>
StringSplit(const std::string &s, const std::string &sep);

/** Convert FP-number to a string as decimal number with fixed number of decimal places after
 * the decimal point (defined by `precision` argument).
 */
std::string
ToFixedString(double d, int precision);

bool
StringStartsWith(std::string_view s, std::string_view prefix);

std::string
ToHex(const void *data, size_t size);

#endif /* INCLUDE_STRING_UTILS_H */
