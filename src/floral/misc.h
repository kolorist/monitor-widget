#pragma once

#include "stdaliases.h"

#ifndef TEST_BIT
#  define TEST_BIT(target, bitmask) (target & bitmask)
#endif
#ifndef TEST_BIT_BOOL
#  define TEST_BIT_BOOL(target, bitmask) ((target & bitmask) != 0)
#endif
#ifndef SET_BIT
#  define SET_BIT(target, bitmask) (target = target | bitmask)
#endif
#ifndef CLEAR_BIT
#  define CLEAR_BIT(target, bitmask) (target = target & (~bitmask))
#endif

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define token_join_impl(x, y) x##y
#define token_join(x, y) token_join_impl(x, y)
#define array_length(arr) (sizeof(arr)/sizeof(arr[0]))
// NOLINTEND(cppcoreguidelines-macro-usage)

#ifndef MARK_UNUSED
#  define MARK_UNUSED(expr) \
      do                    \
      {                     \
          (void)(expr);     \
      } while (0)
#endif

// ----------------------------------------------------------------------------

f32 to_radians(const f32 i_degree);
bool is_aligned(const_voidptr i_addr, const size i_alignment);
bool is_aligned(const aptr i_aptr, const size i_alignment);
size align_size(const size i_size, const size i_alignment);
size align_size_pow2(const size i_size, const size i_alignment);
aptr align_aptr(const aptr i_addr, const size i_alignment);
voidptr align_addr(voidptr i_addr, const size i_alignment);

u32 prev_pow2(u32 i_value);
u64 prev_pow2(u64 i_value);
u32 next_pow2(u32 i_value);
u64 next_pow2(u64 i_value);

#if !defined(math_min)
#  define math_min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#if !defined(math_max)
#  define math_max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#if !defined(math_clamp)
#  define math_clamp(a, x, b) (((x) < (a)) ? (a) : ((x) > (b)) ? (b) : (x))
#endif

f32 mathf_smoothstep(const f32 edge0, const f32 edge1, f32 x);
f32 mathf_abs(const f32 i_value);
f32 mathf_pow(const f32 i_base, const f32 i_exponent);
f32 mathf_round(const f32 i_value);
f32 mathf_floor(const f32 i_value);
f32 mathf_ceil(const f32 i_value);
f32 mathf_log2(const f32 i_value);
f32 mathf_sqrt(const f32 i_value);
f32 mathf_sin(const f32 i_radians);
f32 mathf_cos(const f32 i_radians);
f32 mathf_acos(const f32 i_value);
f32 mathf_tan(const f32 i_radians);

// https://en.cppreference.com/w/c/string/byte/memccpy
voidptr mem_ccopy(voidptr i_dst, const_voidptr i_src, s32 i_chr, size i_len);
void mem_copy(voidptr __restrict i_dst, const_voidptr __restrict i_src, size i_len);
void mem_fill(voidptr __restrict i_dst, const s32 i_value, size i_len);
s32 mem_compare(const_voidptr __restrict i_a, const_voidptr __restrict i_b, size i_len);

void simd_mem_copy(voidptr __restrict i_dst, const_voidptr __restrict i_src, size i_len);
