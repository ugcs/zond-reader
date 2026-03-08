#ifndef INCLUDE_ENDIAN_DEF_H_
#define INCLUDE_ENDIAN_DEF_H_

/**
 * @file endian.h
 *
 * Definitions to work with byte order. There is "endian.h" file present on
 * some systems but it is not standardized and thus not cross-platform.
 */

#include <defs.h>
#include <cstdint>

/** Check if the system is little-endian. */
constexpr bool
IsSystemLe()
{
    return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
}

/** Check if the system is big-endian. */
constexpr bool
IsSystemBe()
{
    return !IsSystemLe();
}


/** Swap bytes in 16-bits integer value. */
#define BSWAP16(__x)    __builtin_bswap16(__x)
/** Swap bytes in 32-bits integer value. */
#define BSWAP32(__x)    __builtin_bswap32(__x)
/** Swap bytes in 64-bits integer value. */
#define BSWAP64(__x)    __builtin_bswap64(__x)


/** Stub for easier conversion functions generalization. */
template <typename T>
constexpr T
ConvertBe8(T x)
{
    return x;
}

/** Convert 16 bits value byte order from BE to host byte order and vice versa.
 * Template parameters:
 * - *T* 16-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertBe16(T x)
{
    return IsSystemBe() ? x : BSWAP16(x);
}

/** Convert 32 bits value byte order from BE to host byte order and vice versa.
 * Template parameters:
 * - *T* 32-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertBe32(T x)
{
    return IsSystemBe() ? x : BSWAP32(x);
}

/** Convert 64 bits value byte order from BE to host byte order and vice versa.
 * Template parameters:
 * - *T* 64-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertBe64(T x)
{
    return IsSystemBe() ? x : BSWAP64(x);
}

/** Stub for easier conversion functions generalization. */
template <typename T>
constexpr T
ConvertLe8(T x)
{
    return x;
}

/** Convert 16 bits value byte order from LE to host byte order and vice versa.
 * Template parameters:
 * - *T* 16-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertLe16(T x)
{
    return IsSystemLe() ? x : BSWAP16(x);
}

/** Convert 32 bits value byte order from LE to host byte order and vice versa.
 * Template parameters:
 * - *T* 32-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertLe32(T x)
{
    return IsSystemLe() ? x : BSWAP32(x);
}

/** Convert 64 bits value byte order from LE to host byte order and vice versa.
 * Template parameters:
 * - *T* 64-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertLe64(T x)
{
    return IsSystemLe() ? x : BSWAP64(x);
}

/** Stub for easier conversion functions generalization. */
template <typename T>
constexpr T
ConvertNh8(T x)
{
    return x;
}

/** Convert 16 bits value byte order from network to host byte order and vice
 * versa.
 * Template parameters:
 * - *T* 16-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertNh16(T x)
{
    return ConvertBe16(x);
}

/** Convert 32 bits value byte order from network to host byte order and vice
 * versa.
 * Template parameters:
 * - *T* 32-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertNh32(T x)
{
    return ConvertBe32(x);
}

/** Convert 64 bits value byte order from network to host byte order and vice
 * versa.
 * Template parameters:
 * - *T* 64-bits integer type.
 * @param x Value to convert.
 */
template <typename T>
constexpr T
ConvertNh64(T x)
{
    return ConvertBe64(x);
}

/** Definitions for byte order conversions for all integer types. */
#define __BO_INT(__typeSize)     int ## __typeSize ## _t
#define __BO_UINT(__typeSize)    uint ## __typeSize ## _t

#define __DEF_BO_CONV(__typeSize, __typeName) \
    /** Convert value from network to host byte order. */ \
    constexpr __typeName \
    Ntoh(__typeName x) \
    { \
        return ConvertNh ## __typeSize (x); \
    } \
    \
    /** Convert value from host to network byte order. */ \
    constexpr __typeName \
    Hton(__typeName x) \
    { \
        return ConvertNh ## __typeSize (x); \
    } \
    \
    /** Convert value from LE to host byte order and vice versa. */ \
    constexpr __typeName \
    Le(__typeName x) \
    { \
        return ConvertLe ## __typeSize (x); \
    } \
    \
    /** Convert value from BE to host byte order and vice versa. */ \
    constexpr __typeName \
    Be(__typeName x) \
    { \
        return ConvertBe ## __typeSize (x); \
    } \

#define _DEF_BO_CONV(__typeSize) \
    __DEF_BO_CONV(__typeSize, __BO_INT(__typeSize)) \
    __DEF_BO_CONV(__typeSize, __BO_UINT(__typeSize))

_DEF_BO_CONV(8)
_DEF_BO_CONV(16)
_DEF_BO_CONV(32)
_DEF_BO_CONV(64)

/* Aliasing rules violation required for FP-numbers byte-order conversion. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

/** Convert float type from host to network format. */
inline float
Hton(float x)
{
    static_assert(sizeof(float) == sizeof(uint32_t));
    uint32_t _x = ConvertBe32(reinterpret_cast<uint32_t &>(x));
    return reinterpret_cast<float &>(_x);
}

/** Convert float type from network to host format. */
inline float
Ntoh(float x)
{
    return Hton(x);
}

/** Convert double type from host to network format. */
inline double
Hton(double x)
{
    static_assert(sizeof(double) == sizeof(uint64_t));
    uint64_t _x = ConvertBe64(reinterpret_cast<uint64_t &>(x));
    return reinterpret_cast<double &>(_x);
}

/** Convert double type from network to host format. */
inline double
Ntoh(double x)
{
    return Hton(x);
}

/** Convert float type from LE to host format. */
inline float
Le(float x)
{
    static_assert(sizeof(float) == sizeof(uint32_t));
    uint32_t _x = ConvertLe32(reinterpret_cast<uint32_t &>(x));
    return reinterpret_cast<float &>(_x);
}

/** Convert float type from BE to host format. */
inline float
Be(float x)
{
    static_assert(sizeof(float) == sizeof(uint32_t));
    uint32_t _x = ConvertBe32(reinterpret_cast<uint32_t &>(x));
    return reinterpret_cast<float &>(_x);
}

/** Convert float type from LE to host format. */
inline double
Le(double x)
{
    static_assert(sizeof(double) == sizeof(uint64_t));
    uint64_t _x = ConvertLe64(reinterpret_cast<uint64_t &>(x));
    return reinterpret_cast<double &>(_x);
}

/** Convert float type from BE to host format. */
inline double
Be(double x)
{
    static_assert(sizeof(double) == sizeof(uint64_t));
    uint64_t _x = ConvertBe64(reinterpret_cast<uint64_t &>(x));
    return reinterpret_cast<double &>(_x);
}

#pragma GCC diagnostic pop


namespace internal {

/** Helper class for LE-order conversions. */
class LeConverter {
public:
    template <typename T>
    static constexpr T
    Convert(T value)
    {
        return Le(value);
    }
};

/** Helper class for BE-order conversions. */
class BeConverter {
public:
    template <typename T>
    static constexpr T
    Convert(T value)
    {
        return Be(value);
    }
};

} /* namespace internal */


/** Helper class for byte-order-dependent value representation. */
template <typename T, class Converter>
class BoValue {
public:
    /** Construct value.
     *
     * @param value Value in host byte order.
     */
    BoValue(T value = 0):
        value(Converter::Convert(value))
    {}

    /** Assign new value.
     *
     * @param value Value in host byte order.
     */
    BoValue &
    operator =(T value)
    {
        this->value = Converter::Convert(value);
        return *this;
    }

    /** Cast to underlying type.
     *
     * @return Value in host byte order.
     */
    operator T() const
    {
        return Converter::Convert(value);
    }

    /** Get the value of underlying type.
     *
     * @return Value in host byte order.
     */
    T
    Get() const
    {
        return Converter::Convert(value);
    }

    /** Interpret byte buffer as a storage for underlying type and
     * return host byte order value. Caller is responsible for the size of
     * the input buffer.
     *
     * @param buffer Input buffer with original value.
     * @return Value in host byte order.
     */
    static T
    Get(const void *buffer)
    {
        return *static_cast<const BoValue *>(buffer);
    }

    /** Save value given in host order to byte buffer.
     * Caller is responsible for the size of the buffer.
     *
     * @param buffer Output buffer to save value to.
     * @param value value in host byte order.
     */
    static void
    Set(void *buffer, const T value)
    {
        *(static_cast<BoValue *>(buffer)) = value;
    }

    uint8_t *
    Bytes()
    {
        return reinterpret_cast<uint8_t *>(&value);
    }

    const uint8_t *
    Bytes() const
    {
        return reinterpret_cast<const uint8_t *>(&value);
    }

private:
    /** Stored value (in wire byte order). */
    T value;
} PACKED;


/** Little-endian value wrapper.
 * @param T Underlying primitive type.
 */
template <typename T>
using LeValue = BoValue<T, internal::LeConverter>;

/** Big-endian value wrapper.
 * @param T Underlying primitive type.
 */
template <typename T>
using BeValue = BoValue<T, internal::BeConverter>;

/** Standard primitive types for little-endian byte order. */
typedef LeValue<int8_t> LeInt8;
typedef LeValue<uint8_t> LeUint8;
typedef LeValue<int16_t> LeInt16;
typedef LeValue<uint16_t> LeUint16;
typedef LeValue<int32_t> LeInt32;
typedef LeValue<uint32_t> LeUint32;
typedef LeValue<int64_t> LeInt64;
typedef LeValue<uint64_t> LeUint64;
typedef LeValue<float> LeFloat;
typedef LeValue<double> LeDouble;


/** Standard primitive types for big-endian byte order. */
typedef BeValue<int8_t> BeInt8;
typedef BeValue<uint8_t> BeUint8;
typedef BeValue<int16_t> BeInt16;
typedef BeValue<uint16_t> BeUint16;
typedef BeValue<int32_t> BeInt32;
typedef BeValue<uint32_t> BeUint32;
typedef BeValue<int64_t> BeInt64;
typedef BeValue<uint64_t> BeUint64;
typedef BeValue<float> BeFloat;
typedef BeValue<double> BeDouble;


#endif /* INCLUDE_ENDIAN_DEF_H_ */
