#include "misc.h"

#include "assert.h"

#include <math.h>
#include <string.h>
#include <immintrin.h>

// ----------------------------------------------------------------------------

f32 to_radians(const f32 i_degree)
{
    return (i_degree / 180.0f * MATH_PI);
}

bool is_aligned(const_voidptr i_addr, const size i_alignment)
{
    return ((aptr)i_addr & (i_alignment - 1)) == 0;
}

bool is_aligned(const aptr i_aptr, const size i_alignment)
{
    return (i_aptr & (i_alignment - 1)) == 0;
}

size align_size(const size i_size, const size i_alignment)
{
    if ((i_size & (i_alignment - 1)) == 0)
    {
        return i_size;
    }

    return i_size + (i_alignment - (i_size & (i_alignment - 1)));
}

size align_size_pow2(const size i_size, const size i_alignment)
{
    return (i_size + i_alignment - 1) & ~(i_alignment - 1);
}

aptr align_aptr(const aptr i_addr, const size i_alignment)
{
    if ((i_addr & (i_alignment - 1)) == 0)
    {
        return i_addr;
    }

    return i_addr + aptr(i_alignment - (i_addr & (i_alignment - 1)));
}

voidptr align_addr(voidptr i_addr, const size i_alignment)
{
    return (voidptr)align_aptr((aptr)i_addr, i_alignment); // NOLINT
}

u32 prev_pow2(u32 i_value)
{
    i_value = i_value | (i_value >> 1);
    i_value = i_value | (i_value >> 2);
    i_value = i_value | (i_value >> 4);
    i_value = i_value | (i_value >> 8);
    i_value = i_value | (i_value >> 16);
    return i_value - (i_value >> 1);
}

u64 prev_pow2(u64 i_value)
{
    i_value = i_value | (i_value >> 1);
    i_value = i_value | (i_value >> 2);
    i_value = i_value | (i_value >> 4);
    i_value = i_value | (i_value >> 8);
    i_value = i_value | (i_value >> 16);
    i_value = i_value | (i_value >> 32);
    return i_value - (i_value >> 1);
}

u32 next_pow2(u32 i_value)
{
    i_value = i_value | (i_value >> 1);
    i_value = i_value | (i_value >> 2);
    i_value = i_value | (i_value >> 4);
    i_value = i_value | (i_value >> 8);
    i_value = i_value | (i_value >> 16);
    return i_value + 1;
}

u64 next_pow2(u64 i_value)
{
    i_value = i_value | (i_value >> 1);
    i_value = i_value | (i_value >> 2);
    i_value = i_value | (i_value >> 4);
    i_value = i_value | (i_value >> 8);
    i_value = i_value | (i_value >> 16);
    i_value = i_value | (i_value >> 32);
    return i_value + 1;
}

f32 mathf_smoothstep(const f32 edge0, const f32 edge1, f32 x)
{
    x = math_clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

f32 mathf_abs(const f32 i_value)
{
    // TODO: implement me!
    return (f32)fabs(i_value);
}

f32 mathf_pow(const f32 i_base, const f32 i_exponent)
{
    // TODO: implement me!
    return powf(i_base, i_exponent);
}

f32 mathf_round(const f32 i_value)
{
    // TODO: implement me!
    return roundf(i_value);
}

f32 mathf_floor(const f32 i_value)
{
    // TODO: implement me!
    return floorf(i_value);
}

f32 mathf_ceil(const f32 i_value)
{
    // TODO: implement me!
    return ceilf(i_value);
}

f32 mathf_log2(const f32 i_value)
{
    // TODO: implement me!
    return log2f(i_value);
}

f32 mathf_sqrt(const f32 i_value)
{
    // TODO: implement me!
    return sqrtf(i_value);
}

f32 mathf_sin(const f32 i_radians)
{
    // TODO: implement me!
    return sinf(i_radians);
}

f32 mathf_cos(const f32 i_radians)
{
    // TODO: implement me!
    return cosf(i_radians);
}

f32 mathf_acos(const f32 i_value)
{
    // TODO: implement me!
    return acosf(i_value);
}

f32 mathf_tan(const f32 i_radians)
{
    // TODO: implement me!
    return tanf(i_radians);
}

voidptr mem_ccopy(voidptr i_dst, const_voidptr i_src, s32 i_chr, size i_len)
{
    if (i_len)
    {
        u8* tp = (u8*)i_dst;
        const u8* fp = (const u8*)i_src;
        u8 uc = (u8)i_chr;
        do
        {
            if ((*tp++ = *fp++) == uc) // NOLINT
            {
                return tp;
            }
        } while (--i_len != 0);
    }
    return nullptr;
}

void mem_fill(voidptr __restrict i_dst, const s32 i_value, size i_len)
{
    // FIXME: replace this?
    memset(i_dst, i_value, i_len);
}

s32 mem_compare(const_voidptr __restrict i_a, const_voidptr __restrict i_b, size i_len)
{
    // FIXME: replace this?
    return memcmp(i_a, i_b, i_len);
}

void mem_copy(voidptr __restrict i_dst, const_voidptr __restrict i_src, size i_len)
{
    // FIXME: this is a very slow version of memcpy
    p8 dst = (p8)i_dst;
    const p8 src = (const p8)i_src;

    for (size i = 0; i < i_len; ++i)
    {
        dst[i] = src[i];
    }
}

void simd_mem_copy(voidptr __restrict i_dst, const_voidptr __restrict i_src, size i_len)
{
    /*
     * AVX - ymm register:
     *  * size: 256 bit ~ 4x f64,s64,u64 ~ 8x f32,s32,u32 ~ 32 bytes
     *  * NOTE: this function will perform worse than memcpy when compiling in Debug (-O0) mode,
     *          because the compiler will only use the ymm0 register for s0, s1, s2, s3, because
     *          it needs those variables to be in the stack so Watch window can inspect their values.
     *          on Optimize (-O2) mode, this performs same with memcpy, both used AVX
     */
    FLORAL_ASSERT_MSG(is_aligned(i_dst, 32), "i_dst must be 32-byte aligned");
    FLORAL_ASSERT_MSG(is_aligned(i_src, 32), "i_src must be 32-byte aligned");

    const __m256i* __restrict srcData = (const __m256i* __restrict)i_src;
    __m256i* __restrict dstData = (__m256i* __restrict)i_dst;

    // simd copy all 32-byte blocks
    size numBlocks = i_len >> 5;
    size blockIdx = 0;
    switch (numBlocks % 4)
    {
    case 3:
    {
        __m256i s = _mm256_load_si256(&srcData[blockIdx]);
        _mm256_stream_si256(&dstData[blockIdx], s);
        blockIdx++; // fallthrough
    }
    case 2:
    {
        __m256i s = _mm256_load_si256(&srcData[blockIdx]);
        _mm256_stream_si256(&dstData[blockIdx], s);
        blockIdx++; // fallthrough
    }
    case 1:
    {
        __m256i s = _mm256_load_si256(&srcData[blockIdx]);
        _mm256_stream_si256(&dstData[blockIdx], s);
        blockIdx++; // fallthrough
    }
    default:
    {
        for (; blockIdx < numBlocks; blockIdx += 4)
        {
            _mm_prefetch((c8*)&srcData[blockIdx + 4], _MM_HINT_NTA);
            __m256i s0 = _mm256_load_si256(&srcData[blockIdx]);
            __m256i s1 = _mm256_load_si256(&srcData[blockIdx + 1]);
            __m256i s2 = _mm256_load_si256(&srcData[blockIdx + 2]);
            __m256i s3 = _mm256_load_si256(&srcData[blockIdx + 3]);
            _mm256_stream_si256(&dstData[blockIdx], s0);
            _mm256_stream_si256(&dstData[blockIdx + 1], s1);
            _mm256_stream_si256(&dstData[blockIdx + 2], s2);
            _mm256_stream_si256(&dstData[blockIdx + 3], s3);
        }
        break;
    }
    }
    FLORAL_ASSERT(blockIdx == numBlocks);

    // scalar copy remaining bytes
    size remaining = i_len - (numBlocks << 5);
    FLORAL_ASSERT(remaining >= 0);
    if (remaining > 0)
    {
        const u8* rSrc = (p8)&srcData[blockIdx];
        p8 rDst = (p8)&dstData[blockIdx];
        for (size i = 0; i < remaining; i++)
        {
            *rDst++ = *rSrc++;
        }
    }

    _mm_mfence();
}
