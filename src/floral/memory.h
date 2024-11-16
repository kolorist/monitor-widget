#pragma once

#include "stdaliases.h"

// ----------------------------------------------------------------------------

#ifndef SIZE_KB
#  define SIZE_KB(X) (X * 1024ull)
#endif
#ifndef SIZE_MB
#  define SIZE_MB(X) (SIZE_KB(X) * 1024ull)
#endif
#ifndef SIZE_GB
#  define SIZE_GB(X) (SIZE_MB(X) * 1024ull)
#endif

#ifndef TO_KB
#  define TO_KB(X) (X >> 10)
#endif
#ifndef TO_MB
#  define TO_MB(X) (TO_KB(X) >> 10)
#endif

// ----------------------------------------------------------------------------

struct alloc_header_t
{
    alloc_header_t* prev;
    alloc_header_t* next;
    p8 marker;
    size frameSize;
    size dataSize;
    size alignment;
};

struct allocator_t
{
    voidptr baseAddress;
    size capacity;
    u32 allocCount;
    u32 freeCount;
    size usedBytes;
    size effectiveBytes;

    const_cstr name;
};

struct linear_allocator_t
{
    allocator_t base;

    p8 marker;
    alloc_header_t* lastAlloc;
};

struct freelist_allocator_t
{
    allocator_t base;

    alloc_header_t* firstFreeBlock;
    alloc_header_t* lastAlloc;
};

struct arena_t
{
    p8 baseAddress;
    aptr marker;
    size capacity;
    size alignment;

    linear_allocator_t* parent;
};

struct scratch_region_t
{
    arena_t* arena;
    aptr origin;
};

///////////////////////////////////////////////////////////////////////////////

// create a new linear allocator by malloc-ing from the heap
linear_allocator_t create_linear_allocator(const_cstr i_name, const size i_bytes);
// create a new linear allocator using placement memory address
linear_allocator_t create_linear_allocator(const_cstr i_name, voidptr i_baseAddress, const size i_bytes);
// create a new linear allocator as a child of a parent linear allocator
linear_allocator_t create_linear_allocator(linear_allocator_t* i_parent, const_cstr i_name, const size i_bytes);
void initialize_allocator(const_cstr i_name, const size i_bytes, linear_allocator_t* const io_allocator);
void initialize_allocator(const_cstr i_name, voidptr i_baseAddress, const size i_bytes, linear_allocator_t* const io_allocator);
void allocator_reset(linear_allocator_t* io_allocator);
void allocator_destroy(linear_allocator_t* io_allocator);
void allocator_destroy(linear_allocator_t* i_parent, linear_allocator_t* i_child);
voidptr allocator_alloc(linear_allocator_t* const i_allocator, const size i_bytes);
voidptr allocator_alloc(linear_allocator_t* const i_allocator, const size i_bytes, const size i_alignment);
voidptr allocator_realloc(linear_allocator_t* const i_allocator, voidptr i_data, const size i_newBytes);
voidptr allocator_realloc(linear_allocator_t* const i_allocator, voidptr i_data, const size i_newBytes, const size i_alignment);
void allocator_free(linear_allocator_t* const i_allocator, voidptr i_data);

// create a new freelist allocator by malloc-ing from the heap
freelist_allocator_t create_freelist_allocator(const_cstr i_name, const size i_bytes);
// create a new freelist allocator using placement memory address
freelist_allocator_t create_freelist_allocator(const_cstr i_name, voidptr i_baseAddress, const size i_bytes);
// create a new freelist allocator as a child of a parent linear allocator
freelist_allocator_t create_freelist_allocator(linear_allocator_t* const i_parent, const_cstr i_name, const size i_bytes);
void initialize_allocator(const_cstr i_name, const size i_bytes, freelist_allocator_t* const io_allocator);
void initialize_allocator(const_cstr i_name, voidptr i_baseAddress, const size i_bytes, freelist_allocator_t* const io_allocator);
void allocator_reset(freelist_allocator_t* const io_allocator);
void allocator_destroy(freelist_allocator_t* const io_allocator);
void allocator_destroy(freelist_allocator_t* const io_allocator);
void allocator_destroy(linear_allocator_t* const i_parent, freelist_allocator_t* const i_child);
voidptr allocator_alloc(freelist_allocator_t* const i_allocator, const size i_bytes);
voidptr allocator_alloc(freelist_allocator_t* const i_allocator, const size i_bytes, const size i_alignment);
voidptr allocator_realloc(freelist_allocator_t* const i_allocator, voidptr i_data, const size i_newBytes);
voidptr allocator_realloc(freelist_allocator_t* const i_allocator, voidptr i_data, const size i_newBytes, const size i_alignment);
void allocator_free(freelist_allocator_t* const i_allocator, voidptr i_data);

arena_t create_arena(linear_allocator_t* const i_allocator, const size i_bytes);
arena_t create_arena(voidptr i_baseAddress, const size i_bytes);
void arena_reset(arena_t* const i_arena);
void arena_destroy(arena_t* const i_arena);
aptr arena_tellp(arena_t* const i_arena);
voidptr arena_push(arena_t* const i_arena, const size i_bytes);
voidptr arena_push(arena_t* const i_arena, const size i_bytes, const size i_alignment);
void arena_pop_to(arena_t* const i_arena, const aptr i_pos);
void arena_pop(arena_t* const i_arena, const size i_bytes);
bool arena_contain(arena_t* const i_arena, const_voidptr i_ptr);

scratch_region_t scratch_begin(arena_t* const i_arena);
void scratch_end(scratch_region_t* const i_scratch);

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define allocator_allocate_pod(allocator, type) \
    (type*)allocator_alloc((allocator), sizeof(type))
#define arena_push_pod(arena, type) \
    (type*)arena_push((arena), sizeof(type))
#define arena_push_pod_aligned(arena, type, alignment) \
    (type*)arena_push((arena), sizeof(type), (alignment))
#define arena_push_podarr(arena, type, count) \
    (type*)arena_push((arena), (count) * sizeof(type))
#define arena_push_podarr_aligned(arena, type, count, alignment) \
    (type*)arena_push((arena), (count) * sizeof(type), (alignment))
#define arena_push_obj(arena, type, ...) \
    ::new (arena_push((arena), sizeof(type)))(type)(__VA_ARGS__)
#define arena_push_objarr(arena, type, count) \
    ::new (arena_push((arena), (count) * sizeof(type))) type[(count)]
// NOLINTEND(cppcoreguidelines-macro-usage)
