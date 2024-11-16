#include "thread_context.h"

///////////////////////////////////////////////////////////////////////////////

thread_local thread_context_t* s_tlThreadContext = nullptr;

void thread_set_context(thread_context_t* const i_ctx)
{
    s_tlThreadContext = i_ctx;
    s_tlThreadContext->arena = create_arena(&s_tlThreadContext->allocator, SIZE_KB(512));
}

thread_context_t* thread_get_context()
{
    if (s_tlThreadContext == nullptr)
    {
        linear_allocator_t threadContextAllocator = create_linear_allocator("temporary thread context allocator", SIZE_MB(4));
        thread_context_t* threadContext = (thread_context_t*)allocator_alloc(&threadContextAllocator, sizeof(thread_context_t));
        threadContext->allocator = threadContextAllocator;
        thread_set_context(threadContext);
    }
    return s_tlThreadContext;
}

arena_t thread_acquire_arena(const size i_bytes)
{
    thread_context_t* const tctx = thread_get_context();
    return create_arena(&tctx->allocator, i_bytes);
}

void thread_release_arena(arena_t* const i_arena)
{
    return arena_destroy(i_arena);
}

scratch_region_t thread_scratch_begin()
{
    thread_context_t* const tctx = thread_get_context();
    return scratch_begin(&tctx->arena);
}

void thread_scratch_end(scratch_region_t* const i_scratch)
{
    return scratch_end(i_scratch);
}
