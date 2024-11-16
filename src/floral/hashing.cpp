#include "hashing.h"

#include "assert.h"
#include "string_utils.h"

u32 compute_crc32(const_cstr i_value)
{
    static constexpr s32 k_CRCWidth = 32;
    static constexpr s32 k_CRCTopBit = 1 << (k_CRCWidth - 1);
    static constexpr s32 k_Polynomial = 0xD8;

    const size strLen = strlen(i_value);
    s32 remainder = 0;

    for (size byte = 0; byte < strLen; byte++)
    {
        remainder ^= (i_value[byte] << (k_CRCWidth - 8));
        for (u8 bit = 8; bit > 0; bit--)
        {
            if (remainder & k_CRCTopBit)
            {
                remainder = (remainder << 1) ^ k_Polynomial;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    return (u32)remainder;
}

u32 compute_murmur_aligned32(const_voidptr i_buffer, const size i_size, const u32 i_seed)
{
    FLORAL_ASSERT(i_buffer != nullptr);
    FLORAL_ASSERT(((aptr)i_buffer & 0x3) == 0);
    const u8* buffer = (u8*)i_buffer;

    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.
    const u32 m = 0x5bd1e995;
    const s32 r = 24;

    // Initialize the hash to a 'random' value
    u32 size = (u32)i_size;
    u32 h = i_seed ^ u32(size);

    // Mix 4 bytes at a time into the hash
    while (size >= 4)
    {
        u32 k = *(u32*)buffer;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        buffer += 4;
        size -= 4;
    }

    // Handle the last few bytes of the input array
    switch (size)
    {
    case 3:
        h ^= (buffer[2]) << 16;
    case 2:
        h ^= (buffer[1]) << 8;
    case 1:
        h ^= (buffer[0]);
        h *= m;
    default:
        break;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

u32 combine_hash(u32 i_lhs, u32 i_rhs)
{
    i_lhs ^= i_rhs + 0x9e3779b9 + (i_lhs << 6) + (i_lhs >> 2);
    return i_lhs;
}
