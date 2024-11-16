#include "configs.h"

#include "utils.h"

static CFGDict s_configs;

void CFGInitialize(linear_allocator_t* i_allocator, file_system_t* const i_fileSystem)
{
    s_configs.arena = create_arena(i_allocator, SIZE_KB(16));
    s_configs.fileGroup = file_system_find_all_files(i_fileSystem, tstr_literal(LITERAL("configs")), tstr_literal(LITERAL("dat")));
    s_configs.configs = arena_create_array(&s_configs.arena, CFGEntry, (u8)CFGKey::KeysCount);
    s_configs.configs.size = (ssize)CFGKey::KeysCount;
    for (ssize i = 0; i < s_configs.configs.size; i++)
    {
        s_configs.configs[i].type = CFGValueType::Undefined;
        s_configs.configs[i].data = nullptr;
    }

    file_handle_t fh = file_ropen(&s_configs.fileGroup, tstr_literal(LITERAL("settings.dat")));
    if (!fh.hasErrors)
    {
        file_read(fh, s_configs.configs.size * sizeof(CFGEntry), s_configs.configs.data);
        file_close(&fh);
    }

    CFGSetBool(CFGKey::StartOnBoot, OSGetStartOnBoot());
}

bool CFGGetBool(const CFGKey i_key)
{
    if (s_configs.configs[(u8)i_key].type == CFGValueType::Boolean &&
        s_configs.configs[(u8)i_key].data != nullptr)
    {
        return (bool)s_configs.configs[(u8)i_key].data;
    }

    return false;
}

void CFGSetBool(const CFGKey i_key, const bool i_value)
{
    if (s_configs.configs[(u8)i_key].type == CFGValueType::Undefined ||
        s_configs.configs[(u8)i_key].type == CFGValueType::Boolean)
    {
        s_configs.configs[(u8)i_key].type = CFGValueType::Boolean;
        s_configs.configs[(u8)i_key].data = (voidptr)i_value;
    }

    file_handle_t fh = file_wopen(&s_configs.fileGroup, tstr_literal(LITERAL("settings.dat")));
    file_write(fh, s_configs.configs.data, s_configs.configs.size * sizeof(CFGEntry));
    file_close(&fh);
}
