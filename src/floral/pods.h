#pragma once

#include "misc.h"
#include "stdaliases.h"

///////////////////////////////////////////////////////////////////////////////

struct buffer_t
{
    voidptr addr;
    size length;
    size capacity;
};

struct const_buffer_t
{
    const_voidptr addr;
    size length;
};

#define arena_create_buffer(arena, cap)     \
    {                                       \
        .addr = arena_push((arena), (cap)), \
        .length = 0,                        \
        .capacity = (cap)                   \
    }

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define buffer_read_start(buffer) \
    {                             \
        p8 reader = (p8)(buffer).addr
#define buffer_read_end() \
    }
#define buffer_read_pod(target, type)         \
    mem_copy((target), reader, sizeof(type)); \
    reader += sizeof(type)
#define buffer_read_podarr(target, type, count)         \
    mem_copy((target), reader, (count) * sizeof(type)); \
    reader += (count) * sizeof(type)

#define buffer_get(type) \
    (*(type*)reader);    \
    reader += sizeof(type)

#define buffer_get_region(len) \
    (reader);                  \
    reader += (len)

// common composed types
#define buffer_read_str8(target, arena)                            \
    size token_join(strLen, __LINE__) = (size)buffer_get(ssize64); \
    *(target) = str8_duplicate((arena), (c8*)reader, token_join(strLen, __LINE__))

// NOLINTEND(cppcoreguidelines-macro-usage)

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define buffer_write_start(buffer) \
    {                              \
        p8 writer = (p8)(buffer).addr + (buffer).length
#define buffer_write_end(buffer)                  \
    (buffer).length = writer - (p8)(buffer).addr; \
    }
#define buffer_write_pod(target, type)        \
    mem_copy(writer, (target), sizeof(type)); \
    writer += sizeof(type)
#define buffer_write(target, size)      \
    mem_copy(writer, (target), (size)); \
    writer += size
#define buffer_write_aligned(target, size, alignment) \
    writer = (p8)align_addr(writer, (alignment));     \
    mem_copy(writer, (target), (size));               \
    writer += size
#define buffer_reserve(type) \
    (type*)writer;           \
    writer += sizeof(type);
// NOLINTEND(cppcoreguidelines-macro-usage)

///////////////////////////////////////////////////////////////////////////////
