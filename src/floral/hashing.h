#pragma once

#include "stdaliases.h"

u32 compute_crc32(const_cstr i_value);
u32 compute_murmur_aligned32(const_voidptr i_buffer, const size i_size, const u32 i_seed);
u32 combine_hash(u32 i_lhs, u32 i_rhs);
