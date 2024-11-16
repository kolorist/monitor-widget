#pragma once

#include "memory.h"
#include "stdaliases.h"

///////////////////////////////////////////////////////////////////////////////

struct thread_context_t
{
    linear_allocator_t allocator;

    arena_t arena;
};

void thread_set_context(thread_context_t* const i_ctx);
thread_context_t* thread_get_context();
arena_t thread_acquire_arena(const size i_bytes);
void thread_release_arena(arena_t* const i_arena);
scratch_region_t thread_scratch_begin();
void thread_scratch_end(scratch_region_t* const i_scratch);
