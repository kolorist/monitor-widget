#include "rng.h"

#include "misc.h"

///////////////////////////////////////////////////////////////////////////////

u32 rng_get_u32(rng_context_t* i_ctx)
{
    u64 oldState = i_ctx->state;
    i_ctx->state = oldState * rng_context_t::k_PCG32_mult + i_ctx->inc;
    u32 xorshifted = (u32)(((oldState >> 18u) ^ oldState) >> 27u);
    u32 rot = (u32)(oldState >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
}

u32 rng_get_u32(rng_context_t* i_ctx, u32 i_modulo)
{
    u32 threshold = (~i_modulo + 1u) % i_modulo;
    while (true)
    {
        u32 r = rng_get_u32(i_ctx);
        if (r >= threshold)
        {
            return r % i_modulo;
        }
    }
}

f32 rng_get_f32(rng_context_t* i_ctx)
{
    return math_min(rng_context_t::k_f32_one_minus_epsilon, f32(rng_get_u32(i_ctx)) * 2.3283064365386963e-10f);
}

f32 rng_get_f32_range(rng_context_t* i_ctx, const f32 i_minInclusive, const f32 i_maxInclusive)
{
    f32 t = rng_get_f32(i_ctx);
    return i_minInclusive + (i_maxInclusive - i_minInclusive) * t;
}

f64 rng_get_f64(rng_context_t* i_ctx)
{
    return math_min(rng_context_t::k_f64_one_minus_epsilon, f64(rng_get_u32(i_ctx)) * 2.3283064365386963e-10f);
}

rng_context_t create_rng(u64 i_sequenceIndex)
{
    rng_context_t rng;
    rng.state = 0u;
    rng.inc = (i_sequenceIndex << 1u) | 1u;
    rng_get_u32(&rng);
    rng.state += rng_context_t::k_PCG32_default_state;
    rng_get_u32(&rng);
    return rng;
}
