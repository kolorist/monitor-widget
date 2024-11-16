#pragma once

#include <floral/stdaliases.h>
#include <floral/container.h>
#include <floral/memory.h>
#include <floral/file_system.h>

enum class CFGKey : u8
{
    ShowInTaskBar = 0,
    StartOnBoot,
    KeysCount
};

enum class CFGValueType : u8
{
    Undefined = 0,
    Boolean,
    Integer
};

struct CFGEntry
{
    CFGValueType type;
    voidptr data;
};

struct CFGDict
{
    file_group_t fileGroup;
    array_t<CFGEntry> configs;
    arena_t arena;
};

void CFGInitialize(linear_allocator_t* i_allocator, file_system_t* const i_fileSystem);
bool CFGGetBool(const CFGKey i_key);
void CFGSetBool(const CFGKey i_key, const bool i_value);
