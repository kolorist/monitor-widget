#pragma once

#include "container.h"
#include "stdaliases.h"

#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////

struct arena_t;

///////////////////////////////////////////////////////////////////////////////

size cstr_length(const_cstr i_str);
cstr cstr_xcopy(cstr i_dst, const size i_dstSize, const_cstr i_src);
s32 cstr_compare(const_cstr i_a, const_cstr i_b);
void cstr_concat(cstr o_dest, const size i_destSize, const_cstr i_src);
s32 cstr_snprintf(cstr o_buffer, const size i_bufferLength, const_cstr i_fmt, ...);
s32 cstr_vsnprintf(cstr o_buffer, const size i_bufferLength, const_cstr i_format, va_list i_args);

wcstr wcstr_duplicate(arena_t* const i_arena, const_wcstr i_str);
size wcstr_length(const_wcstr i_str);
wcstr wcstr_xcopy(wcstr i_dst, const size i_dstSize, const_wcstr i_src);
s32 wcstr_compare(const_wcstr i_a, const_wcstr i_b);
void wcstr_concat(wcstr o_dest, const size i_destSize, const_wcstr i_src);
s32 wcstr_snprintf(wcstr o_buffer, const size i_bufferLength, const_wcstr i_fmt, ...);
s32 wcstr_vsnprintf(wcstr o_buffer, const size i_bufferLength, const_wcstr i_format, va_list i_args);

// return number of character written (not counting the null-terminator)
size to_cstr(const_cstr i_input, cstr o_buffer, const size i_bufferLength);
size to_cstr(const_wcstr i_input, cstr o_buffer, const size i_bufferLength);
// return number of character written (not counting the null-terminator)
size to_wcstr(const_cstr i_input, wcstr o_buffer, const size i_bufferLength);
size to_wcstr(const_wcstr i_input, wcstr o_buffer, const size i_bufferLength);

///////////////////////////////////////////////////////////////////////////////

struct str8 // NOLINT(readability-identifier-naming)
{
    const c8* data;
    size length; // not counting the null-terminator
};

str8 str8_literal(const_cstr i_str);
str8 str8_duplicate(arena_t* const i_arena, const_cstr i_str);
str8 str8_duplicate(arena_t* const i_arena, const str8& i_str);
str8 str8_duplicate(arena_t* const i_arena, const_cstr i_begin, const_cstr i_end);
str8 str8_duplicate(arena_t* const i_arena, const_cstr i_str, const size i_length);
str8 str8_printf(arena_t* const i_arena, const_cstr i_fmt, ...);
str8 str8_vprintf(arena_t* const i_arena, const_cstr i_fmt, va_list i_args);
str8 str8_concat(arena_t* const i_arena, const str8& i_a, const str8& i_b);
s32 str8_compare(const str8& i_a, const str8& i_b);
str8 str8_replace_char(arena_t* const i_arena, const str8& i_str, const c8 i_old, const c8 i_new);
bool str8_starts_with(const str8& i_str, const_cstr i_cstr);
u32 str8_crc32_hash(const str8& i_str);
u32 str8_fnv1a32_hash(const str8& i_str);
s32 str8_to_s32(const str8& i_str);
f32 str8_to_f32(const str8& i_str);

///////////////////////////////////////////////////////////////////////////////

struct str16 // NOLINT(readability-identifier-naming)
{
    const c16* data;
    size length; // not counting the null-terminator
};

str16 str16_literal(const_wcstr i_str);
str16 str16_duplicate(arena_t* const i_arena, const_wcstr i_str);
str16 str16_duplicate(arena_t* const i_arena, const_cstr i_str);
str16 str16_duplicate(arena_t* const i_arena, const str8& i_str);
str16 str16_duplicate(arena_t* const i_arena, const str16& i_str);
str16 str16_duplicate(arena_t* const i_arena, const_wcstr i_begin, const_wcstr i_end);
str16 str16_duplicate(arena_t* const i_arena, const_cstr i_str, const size i_length);
str16 str16_duplicate(arena_t* const i_arena, const_wcstr i_str, const size i_length);
str16 str16_printf(arena_t* const i_arena, const_wcstr i_fmt, ...);
str16 str16_vprintf(arena_t* const i_arena, const_wcstr i_fmt, va_list i_args);
str16 str16_concat(arena_t* const i_arena, const str16& i_a, const str16& i_b);
dll_t<str16> str16_split(arena_t* const i_arena, const str16& i_str, const c16 i_delimiter);
str16 str16_replace_char(arena_t* const i_arena, const str16& i_str, const c16 i_old, const c16 i_new);
u32 str16_crc32_hash(const str16& i_str);
u32 str16_fnv1a32_hash(const str16& i_str);

///////////////////////////////////////////////////////////////////////////////

constexpr str8 k_str8Empty = {
    .data = "",
    .length = 0
};
constexpr str16 k_str16Empty = {
    .data = L"",
    .length = 0
};

///////////////////////////////////////////////////////////////////////////////

#if defined(UNICODE) || defined(_UNICODE)
#  define LITERAL(quote) L##quote
#  define tcstr_duplicate wcstr_duplicate
#  define tcstr_compare wcstr_compare
#  define tcstr_xcopy wcstr_xcopy
#  define tcstr_concat wcstr_concat
#  define tcstr_length wcstr_length
#  define tcstr_vsnprintf wcstr_vsnprintf

#  define tstr_literal str16_literal
#  define tstr_duplicate str16_duplicate
#  define tstr_printf str16_printf
#  define tstr_vprintf str16_vprintf
#  define tstr_concat str16_concat
#  define tstr_split str16_split
#  define tstr_replace_char str16_replace_char
#  define tstr_crc32_hash str16_crc32_hash
#  define tstr_fnv1a32_hash str16_fnv1a32_hash

#  define k_tstrEmpty k_str16Empty
typedef str16 tstr;
#else
#  define LITERAL(quote) quote
#  define tcstr_duplicate cstr_duplicate
#  define tcstr_compare cstr_compare
#  define tcstr_xcopy cstr_xcopy
#  define tcstr_concat cstr_concat
#  define tcstr_length cstr_length
#  define tcstr_vsnprintf cstr_vsnprintf

#  define tstr_literal str8_literal
#  define tstr_duplicate str8_duplicate
#  define tstr_printf str8_printf
#  define tstr_concat str8_concat
#  define tstr_split str8_split
#  define tstr_replace_char str8_replace_char
#  define tstr_crc32_hash str8_crc32_hash
#  define tstr_fnv1a32_hash str8_fnv1a32_hash

#  define k_tstrEmpty k_str8Empty
typedef str8 tstr;
#endif
