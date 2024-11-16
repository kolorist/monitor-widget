#include "string_utils.h"

#include "assert.h"
#include "memory.h"
#include "misc.h"

#include <stdio.h>
#include <string.h>

// ----------------------------------------------------------------------------

size cstr_length(const_cstr i_str)
{
    // TODO
    return strlen(i_str);
}

// `i_dstSize` is size of the `i_dst` buffer, including space for the null-terminator
cstr cstr_xcopy(cstr i_dst, const size i_dstSize, const_cstr i_src)
{
    cstr end = (cstr)mem_ccopy(i_dst, i_src, 0, i_dstSize);
    if (!end && i_dstSize)
    {
        i_dst[i_dstSize - 1] = 0;
    }
    return end;
}

s32 cstr_compare(const_cstr i_a, const_cstr i_b)
{
    // TODO
    return strcmp(i_a, i_b);
}

void cstr_concat(cstr o_dest, const size i_destSize, const_cstr i_src)
{
    // TODO
    strcat_s(o_dest, i_destSize, i_src);
}

s32 cstr_snprintf(cstr o_buffer, const size i_bufferLength, const_cstr i_fmt, ...)
{
    va_list args; // NOLINT(cppcoreguidelines-init-variables)
    va_start(args, i_fmt);
    s32 ret = cstr_vsnprintf(o_buffer, i_bufferLength, i_fmt, args);
    va_end(args);
    return ret;
}

s32 cstr_vsnprintf(cstr o_buffer, const size i_bufferLength, const_cstr i_format, va_list i_args)
{
    // TODO
    return vsnprintf(o_buffer, i_bufferLength, i_format, i_args);
}

wcstr wcstr_duplicate(arena_t* const i_arena, const_wcstr i_str)
{
    size length = wcstr_length(i_str);
    c16* buffer = arena_push_podarr(i_arena, c16, length + 1);
    mem_copy(buffer, i_str, length * sizeof(c16));
    buffer[length] = 0;
    return buffer;
}

size wcstr_length(const_wcstr i_str)
{
    // TODO
    return wcslen(i_str);
}

wcstr wcstr_xcopy(wcstr i_dst, const size i_dstSize, const_wcstr i_src)
{
    if (i_dstSize)
    {
        size len = i_dstSize;
        c16* tp = (c16*)i_dst;
        const c16* fp = (const c16*)i_src;
        do
        {
            if ((*tp++ = *fp++) == 0) // NOLINT
            {
                break;
            }
        } while (--len != 0);

        i_dst[i_dstSize - 1] = 0;
        return tp;
    }
    return nullptr;
}

s32 wcstr_compare(const_wcstr i_a, const_wcstr i_b)
{
    // TODO
    return wcscmp(i_a, i_b);
}

void wcstr_concat(wcstr o_dest, const size i_destSize, const_wcstr i_src)
{
    // TODO
    wcscat_s(o_dest, i_destSize, i_src);
}

s32 wcstr_snprintf(wcstr o_buffer, const size i_bufferLength, const_wcstr i_fmt, ...)
{
    va_list args; // NOLINT(cppcoreguidelines-init-variables)
    va_start(args, i_fmt);
    s32 ret = wcstr_vsnprintf(o_buffer, i_bufferLength, i_fmt, args);
    va_end(args);
    return ret;
}

s32 wcstr_vsnprintf(wcstr o_buffer, const size i_bufferLength, const_wcstr i_format, va_list i_args)
{
    // TODO
    return vswprintf(o_buffer, i_bufferLength, i_format, i_args);
}

size to_cstr(const_wcstr i_input, cstr o_buffer, const size i_bufferLength)
{
    size characterWritten = 0;
    ssize errCode = 0;
    errCode = wcstombs_s(&characterWritten, o_buffer, i_bufferLength, i_input, i_bufferLength - 1);
    FLORAL_ASSERT(errCode >= 0);
    return characterWritten - 1;
}

size to_wcstr(const_cstr i_input, wcstr o_buffer, const size i_bufferLength)
{
    size characterWritten = 0;
    ssize errCode = 0;
    errCode = mbstowcs_s(&characterWritten, o_buffer, i_bufferLength, i_input, i_bufferLength - 1);
    FLORAL_ASSERT(errCode >= 0);
    return characterWritten - 1;
}

///////////////////////////////////////////////////////////////////////////////

str8 str8_literal(const_cstr i_str)
{
    return { .data = i_str, .length = cstr_length(i_str) };
}

str8 str8_duplicate(arena_t* const i_arena, const_cstr i_str)
{
    return str8_duplicate(i_arena, i_str, cstr_length(i_str));
}

str8 str8_duplicate(arena_t* const i_arena, const str8& i_str)
{
    return str8_duplicate(i_arena, i_str.data, i_str.length);
}

str8 str8_duplicate(arena_t* const i_arena, const_cstr i_begin, const_cstr i_end)
{
    return str8_duplicate(i_arena, i_begin, i_end - i_begin);
}

str8 str8_duplicate(arena_t* const i_arena, const_cstr i_str, const size i_length)
{
    c8* buffer = arena_push_podarr(i_arena, c8, i_length + 1);
    mem_copy(buffer, i_str, i_length);
    buffer[i_length] = 0;

    return { .data = buffer, .length = i_length };
}

str8 str8_duplicate(arena_t* const i_arena, const str8* const i_str)
{
    return str8_duplicate(i_arena, i_str->data, i_str->length);
}

str8 str8_printf(arena_t* const i_arena, const_cstr i_fmt, ...)
{
    va_list args, backupArgs; // NOLINT(cppcoreguidelines-init-variables)
    va_start(args, i_fmt);
    va_copy(backupArgs, args);

    const aptr orgP = arena_tellp(i_arena);
    c8* buffer = arena_push_podarr(i_arena, c8, FLORAL_MAX_LOCAL_BUFFER_LENGTH);
    const s32 length = cstr_vsnprintf(buffer, FLORAL_MAX_LOCAL_BUFFER_LENGTH, i_fmt, args);
    FLORAL_ASSERT(length > 0 && length < FLORAL_MAX_LOCAL_BUFFER_LENGTH);
    const s32 requiredStorageSize = length + 1;

    if (requiredStorageSize < FLORAL_MAX_LOCAL_BUFFER_LENGTH)
    {
        aptr newP = arena_tellp(i_arena) - (FLORAL_MAX_LOCAL_BUFFER_LENGTH - requiredStorageSize);
        arena_pop_to(i_arena, newP);
    }
    else if (requiredStorageSize > FLORAL_MAX_LOCAL_BUFFER_LENGTH)
    {
        arena_pop_to(i_arena, orgP);
        buffer = arena_push_podarr(i_arena, c8, requiredStorageSize);
        cstr_vsnprintf(buffer, requiredStorageSize, i_fmt, backupArgs);
    }

    va_end(args);
    va_end(backupArgs);
    return { .data = buffer, .length = (size)length };
}

str8 str8_concat(arena_t* const i_arena, const str8& i_a, const str8& i_b)
{
    size length = i_a.length + i_b.length;

    c8* buffer = arena_push_podarr(i_arena, c8, length + 1);
    mem_copy(buffer, i_a.data, i_a.length);
    mem_copy(buffer + i_a.length, i_b.data, i_b.length);
    buffer[length] = 0;

    return { .data = buffer, .length = length };
}

s32 str8_compare(const str8& i_a, const str8& i_b)
{
    size i = 0, j = 0;
    while (i_a.data[i] == i_b.data[j++])
    {
        if (i++ == i_a.length)
        {
            return 0;
        }
    }
    return (u8)i_a.data[i] - (u8)i_b.data[j - 1];
}

str8 str8_replace_char(arena_t* const i_arena, const str8& i_str, const c8 i_old, const c8 i_new)
{
    size length = i_str.length;
    c8* buffer = arena_push_podarr(i_arena, c8, length + 1);
    cstr_xcopy(buffer, length + 1, i_str.data);
    buffer[length] = 0;
    for (size i = 0; i < length; i++)
    {
        if (buffer[i] == i_old)
        {
            buffer[i] = i_new;
        }
    }
    return { .data = buffer, .length = length };
}

bool str8_starts_with(const str8& i_str, const_cstr i_cstr)
{
    const size len = cstr_length(i_cstr);
    if (len > i_str.length)
    {
        return false;
    }

    return mem_compare(i_str.data, i_cstr, len) == 0;
}

// FIXME: implement CRC32 yourself!
u32 str8_crc32_hash(const str8& i_str)
{
    constexpr s32 crcWidth = 32;
    constexpr s32 crcTopBit = 1 << (crcWidth - 1);
    constexpr s32 polynomial = 0xD8;

    s32 remainder = 0;
    p8 bytes = (p8)i_str.data;
    for (size byte = 0; byte < i_str.length; byte++)
    {
        remainder ^= (bytes[byte] << (crcWidth - 8));
        for (u8 bit = 8; bit > 0; bit--)
        {
            if (remainder & crcTopBit)
            {
                remainder = (remainder << 1) ^ polynomial;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    return (u32)remainder;
}

u32 str8_fnv1a32_hash(const str8& i_str)
{
    const u32 fnvPrime = 0x01000193;
    const u32 fnvOffsetBasis = 0x811c9dc5;

    u32 hash = fnvOffsetBasis;
    p8 bytes = (p8)i_str.data;
    for (size i = 0; i < i_str.length; i++)
    {
        hash ^= bytes[i];
        hash *= fnvPrime;
    }
    return hash;
}

s32 str8_compare(const str8* const i_str, const_cstr i_cstr)
{
    return cstr_compare(i_str->data, i_cstr);
}

s32 str8_to_s32(const str8& i_str)
{
    s32 v = 0;
    for (size i = 0; i < i_str.length; i++)
    {
        FLORAL_ASSERT(i_str.data[i] >= '0' && i_str.data[i] <= '9');
        v *= 10;
        v += i_str.data[i] - '0';
    }
    return v;
}

f32 str8_to_f32(const str8& i_str)
{
    // TODO: implement
    return (f32)atof(i_str.data);
}

///////////////////////////////////////////////////////////////////////////////

str16 str16_literal(const_wcstr i_str)
{
    return { .data = i_str, .length = wcstr_length(i_str) };
}

str16 str16_duplicate(arena_t* const i_arena, const_wcstr i_str)
{
    return str16_duplicate(i_arena, i_str, wcstr_length(i_str));
}

str16 str16_duplicate(arena_t* const i_arena, const_cstr i_str)
{
    const size expectedLength = cstr_length(i_str);
    return str16_duplicate(i_arena, i_str, expectedLength);
}

str16 str16_duplicate(arena_t* const i_arena, const str8& i_str)
{
    return str16_duplicate(i_arena, i_str.data, i_str.length);
}

str16 str16_duplicate(arena_t* const i_arena, const str16& i_str)
{
    return str16_duplicate(i_arena, i_str.data, i_str.length);
}

str16 str16_duplicate(arena_t* const i_arena, const_wcstr i_begin, const_wcstr i_end)
{
    return str16_duplicate(i_arena, i_begin, i_end - i_begin);
}

str16 str16_duplicate(arena_t* const i_arena, const_cstr i_str, const size i_length)
{
    c16* buffer = arena_push_podarr(i_arena, c16, i_length + 1);
    const size length = to_wcstr(i_str, buffer, i_length + 1);
    FLORAL_ASSERT(length == i_length);

    return { .data = buffer, .length = i_length };
}

str16 str16_duplicate(arena_t* const i_arena, const_wcstr i_str, const size i_length)
{
    c16* buffer = arena_push_podarr(i_arena, c16, i_length + 1);
    mem_copy(buffer, i_str, i_length * sizeof(c16));
    buffer[i_length] = 0;

    return { .data = buffer, .length = i_length };
}

str16 str16_printf(arena_t* const i_arena, const_wcstr i_fmt, ...)
{
    va_list args; // NOLINT(cppcoreguidelines-init-variables)
    va_start(args, i_fmt);
    str16 str = str16_vprintf(i_arena, i_fmt, args);
    va_end(args);
    return str;
}

str16 str16_vprintf(arena_t* const i_arena, const_wcstr i_fmt, va_list i_args)
{
    va_list backupArgs; // NOLINT(cppcoreguidelines-init-variables)
    va_copy(backupArgs, i_args);

    const aptr orgP = arena_tellp(i_arena);
    c16* buffer = arena_push_podarr(i_arena, c16, FLORAL_MAX_LOCAL_BUFFER_LENGTH);
    const s32 length = wcstr_vsnprintf(buffer, FLORAL_MAX_LOCAL_BUFFER_LENGTH, i_fmt, i_args);
    FLORAL_ASSERT(length > 0 && length < FLORAL_MAX_LOCAL_BUFFER_LENGTH);
    const s32 requiredStorageSize = length + 1;

    if (requiredStorageSize < FLORAL_MAX_LOCAL_BUFFER_LENGTH)
    {
        aptr newP = arena_tellp(i_arena) - (FLORAL_MAX_LOCAL_BUFFER_LENGTH - requiredStorageSize);
        arena_pop_to(i_arena, newP);
    }
    else if (requiredStorageSize > FLORAL_MAX_LOCAL_BUFFER_LENGTH)
    {
        arena_pop_to(i_arena, orgP);
        buffer = arena_push_podarr(i_arena, c16, requiredStorageSize);
        wcstr_vsnprintf(buffer, requiredStorageSize, i_fmt, backupArgs);
    }

    va_end(backupArgs);
    return { .data = buffer, .length = (size)length };
}

str16 str16_concat(arena_t* const i_arena, const str16& i_a, const str16& i_b)
{
    size length = i_a.length + i_b.length;

    c16* buffer = arena_push_podarr(i_arena, c16, length + 1);
    mem_copy(buffer, i_a.data, i_a.length * sizeof(c16));
    mem_copy(buffer + i_a.length, i_b.data, i_b.length * sizeof(c16));
    buffer[length] = 0;

    return { .data = buffer, .length = length };
}

dll_t<str16> str16_split(arena_t* const i_arena, const str16& i_str, const c16 i_delimiter)
{
    dll_t<str16> strList = create_dll<str16>();
    size i = 0, j = 0;
    for (; j < i_str.length; j++)
    {
        if (i_str.data[j] == i_delimiter)
        {
            size len = j - i;
            dll_t<str16>::node_t* strNode = arena_push_pod(i_arena, dll_t<str16>::node_t);
            strNode->data = str16_duplicate(i_arena, &i_str.data[i], len);
            dll_push_back(&strList, strNode);
            j++;
            i = j;
        }
    }

    if (i != j)
    {
        dll_t<str16>::node_t* strNode = arena_push_pod(i_arena, dll_t<str16>::node_t);
        strNode->data = str16_duplicate(i_arena, &i_str.data[i], j - i + 1);
        dll_push_back(&strList, strNode);
    }
    return strList;
}

str16 str16_replace_char(arena_t* const i_arena, const str16& i_str, const c16 i_old, const c16 i_new)
{
    size length = i_str.length;
    c16* buffer = arena_push_podarr(i_arena, c16, length + 1);
    wcstr_xcopy(buffer, length + 1, i_str.data);
    buffer[length] = 0;
    for (size i = 0; i < length; i++)
    {
        if (buffer[i] == i_old)
        {
            buffer[i] = i_new;
        }
    }
    return { .data = buffer, .length = length };
}

// FIXME: implement CRC32 yourself!
u32 str16_crc32_hash(const str16& i_str)
{
    constexpr s32 crcWidth = 32;
    constexpr s32 crcTopBit = 1 << (crcWidth - 1);
    constexpr s32 polynomial = 0xD8;

    s32 remainder = 0;
    p8 bytes = (p8)i_str.data;
    for (size byte = 0; byte < i_str.length; byte++)
    {
        remainder ^= (bytes[byte] << (crcWidth - 8));
        for (u8 bit = 8; bit > 0; bit--)
        {
            if (remainder & crcTopBit)
            {
                remainder = (remainder << 1) ^ polynomial;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    return (u32)remainder;
}

u32 str16_fnv1a32_hash(const str16& i_str)
{
    const u32 fnvPrime = 0x01000193;
    const u32 fnvOffsetBasis = 0x811c9dc5;

    u32 hash = fnvOffsetBasis;
    p8 bytes = (p8)i_str.data;
    size length = i_str.length * sizeof(c16);
    for (size i = 0; i < length; i++)
    {
        hash ^= bytes[i];
        hash *= fnvPrime;
    }
    return hash;
}
