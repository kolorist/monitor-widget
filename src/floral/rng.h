#pragma once

#include "stdaliases.h"

///////////////////////////////////////////////////////////////////////////////

struct rng_context_t
{
    static constexpr u64 k_PCG32_default_state = 0x853c49e6748fea9bull;
    static constexpr u64 k_PCG32_default_stream = 0xda3e39cb94b95bdbull;
    static constexpr u64 k_PCG32_mult = 0x5851f42d4c957f2dull;

    static constexpr f32 k_f32_one_minus_epsilon = 0.99999994f;
    static constexpr f64 k_f64_one_minus_epsilon = 0.99999999999999989;

    u64 state;
    u64 inc;
};

u32 rng_get_u32(rng_context_t* i_ctx);
u32 rng_get_u32(rng_context_t* i_ctx, u32 i_modulo);

/*
 * Returns a random f32 value in range [0..1]
 */
f32 rng_get_f32(rng_context_t* i_ctx);

/*
 * Returns a random f32 value in range [i_minInclusive..i_maxInclusive]
 */
f32 rng_get_f32_range(rng_context_t* i_ctx, const f32 i_minInclusive, const f32 i_maxInclusive);

/*
 * Returns a random f64 value in range [0..1]
 */
f64 rng_get_f64(rng_context_t* i_ctx);

rng_context_t create_rng(u64 i_sequenceIndex);
