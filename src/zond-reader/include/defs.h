/*
 * Common preprocessor definitions.
 */

#ifndef INCLUDE_DEFS_H_
#define INCLUDE_DEFS_H_

#define CONCAT2(__x, __y)       __x##__y

/** Macro for concatenating identifiers. */
#define CONCAT(__x, __y)        CONCAT2(__x, __y)

#define STR2(__x)               # __x
/** Macro for stringifying identifiers. */
#define STR(__x)                STR2(__x)

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(__array)   (sizeof(__array) / sizeof((__array)[0]))
#endif

/** Specify that a function has format arguments (like printf or scanf). See
 * 'format' attribute description in GCC documentation.
 */
#if defined(__GNUC__) || defined(__clang__)
#define FORMAT(__type, __fmt_idx, __arg_idx)    \
    __attribute__ ((format(__type, __fmt_idx, __arg_idx)))
#else
#define FORMAT(type, fmt_idx, arg_idx)
#endif /* __GNUC__ */

#ifndef _WIN32
#define FORMAT_PRINTF(__fmt_idx, __arg_idx) FORMAT(printf, __fmt_idx, __arg_idx)
#else
#ifndef __USE_MINGW_ANSI_STDIO
#error __USE_MINGW_ANSI_STDIO=1 must be defined via makefile
#endif
#define FORMAT_PRINTF(__fmt_idx, __arg_idx) FORMAT(__MINGW_PRINTF_FORMAT, __fmt_idx, __arg_idx)
#endif


/** Pack structure or class, i.e. do not allow the compiler to insert paddings
 * for members alignment.
 */
#define PACKED      __attribute__((packed))

/** For constants from cmath.h */
#define _USE_MATH_DEFINES

/** Align integer value to the next alignment position. Alignment must be power of 2. */
#define ALIGN2(__x, __alignment)   (((__x) + (__alignment) - 1) & ~((__alignment) - 1))

/** Check if non-null integer value is power of two. */
#define IS_POW2(__x)    ((((__x) - 1) & (__x)) == 0)

#endif /* INCLUDE_DEFS_H_ */
