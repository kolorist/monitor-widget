#include "scripting.h"

#include <floral/file_system.h>
#include <floral/log.h>
#include <floral/misc.h>
#include <floral/time.h>
#include <floral/string_utils.h>
#include <floral/thread_context.h>

static SCRContext s_context;

// ----------------------------------------------------------------------------

void SCRRunFile(const tstr& i_filePath);

// ----------------------------------------------------------------------------

static void* LuaAlloc(void* i_ud, void* i_ptr, size_t i_oldSize, size_t i_newSize)
{
    MARK_UNUSED(i_oldSize);
    freelist_allocator_t* allocator = (freelist_allocator_t*)i_ud;
    if (i_newSize == 0)
    {
        if (i_ptr != nullptr)
        {
            allocator_free(allocator, i_ptr);
        }
    }
    else
    {
        if (i_ptr == nullptr)
        {
            return allocator_alloc(allocator, i_newSize);
        }
        else
        {
            return allocator_realloc(allocator, i_ptr, i_newSize);
        }
    }
    return nullptr;
}

static s32 ScriptingPrint(lua_State* i_vm)
{
    scratch_region_t scratch = thread_scratch_begin();

    LOG_SCOPE(log);
    const s32 nArgs = lua_gettop(i_vm);
    c8* buffer = arena_push_podarr(scratch.arena, c8, FLORAL_MAX_LOCAL_BUFFER_LENGTH);
    buffer[0] = 0;
    cstr_concat(buffer, FLORAL_MAX_LOCAL_BUFFER_LENGTH, "[scripting] ");
    for (s32 i = 1; i <= nArgs; i++)
    {
        cstr_concat(buffer, FLORAL_MAX_LOCAL_BUFFER_LENGTH, lua_tostring(i_vm, i));
    }
    LOG_DEBUG(buffer);

    thread_scratch_end(&scratch);
    return 0;
}

static s32 ScriptingLoadModule(lua_State* i_vm)
{
    LOG_SCOPE(scripting);
    const s32 nArgs = lua_gettop(i_vm);
    FLORAL_ASSERT(nArgs == 1);
    const_cstr modulePath = lua_tostring(i_vm, 1);

    SCRContext* const ctx = (SCRContext*)lua_touserdata(i_vm, lua_upvalueindex(2));
    scratch_region_t scratch = scratch_begin(&ctx->arena);
    SCRRunFile(tstr_duplicate(scratch.arena, modulePath));
    scratch_end(&scratch);

    return 0;
}

// ----------------------------------------------------------------------------

void SCRInitialize(file_system_t* const i_fs, linear_allocator_t* const i_allocator)
{
    s_context.arena = create_arena(i_allocator, SIZE_MB(1));
    s_context.vmAllocator = create_freelist_allocator(i_allocator, "Lua VM allocator", SIZE_MB(2));

    s_context.fileSystem = i_fs;
    s_context.scriptFileGroup = create_file_group(i_fs);

    s_context.entryPoints = create_dll<tstr>();
    s_context.vm = nullptr;
    s_context.ready = true;
}

SCRContext* SCRGetContext()
{
    return &s_context;
}

void SCRLoadVMThread(const tstr& i_subPath)
{
    LOG_SCOPE(scripting);
    file_group_reset(&s_context.scriptFileGroup);
    file_system_find_all_files(s_context.fileSystem, i_subPath, tstr_literal(LITERAL("lua")), &s_context.scriptFileGroup);
    LOG_DEBUG("Script files scanned, found %d files", s_context.scriptFileGroup.fileCount);

    FLORAL_ASSERT(s_context.vm == nullptr);

    s_context.vm = lua_newstate(&LuaAlloc, &s_context.vmAllocator);
    SCRStackGuard guard(s_context.vm);

    // load libraries
    const luaL_Reg luaLibs[] = {
        {             "",   luaopen_base},
        { LUA_TABLIBNAME,  luaopen_table},
        { LUA_STRLIBNAME, luaopen_string},
        {LUA_MATHLIBNAME,   luaopen_math},
    };
    for (u32 i = 0; i < array_length(luaLibs); i++)
    {
        lua_pushcfunction(s_context.vm, luaLibs[i].func);
        lua_pushstring(s_context.vm, luaLibs[i].name);
        lua_call(s_context.vm, 1, 0);
    }

    luaL_Reg hookedFunctions[] = {
        {"print", &ScriptingPrint},
        {nullptr,   nullptr},
    };
    lua_getglobal(s_context.vm, "_G");
    luaL_register(s_context.vm, nullptr, hookedFunctions);
    lua_pop(s_context.vm, 1);
    SCRRegisterFunc(ScriptingLoadModule, "load_module", nullptr);

    s_context.ready = true;
    LOG_DEBUG("VM thread loaded, loading entry points...");

    dll_t<tstr>::node_t* it = nullptr;
    dll_for_each(&s_context.entryPoints, it)
    {
        SCRRunFile(it->data);
    }
}

bool SCRReloadVMThread()
{
    LOG_SCOPE(scripting);
    bool compileSucceed = true;

    scratch_region_t scratch = scratch_begin(&s_context.arena);
    dll_t<tstr>::node_t* it = nullptr;
    dll_for_each(&s_context.entryPoints, it)
    {
        tstr filePath = it->data;
        LOG_DEBUG(LITERAL("Compile script: %s"), filePath.data);

        lua_State* const vm = s_context.vm;
        file_handle_t fh = file_ropen(&s_context.scriptFileGroup, filePath);
        const_buffer_t buffer = file_read_all(fh, scratch.arena);
        file_close(&fh);

        c8* bufferName = arena_push_podarr(scratch.arena, c8, FLORAL_MAX_NAME_LENGTH);
        to_cstr(filePath.data, bufferName, FLORAL_MAX_NAME_LENGTH);

        SCRStackGuard guard(vm);
        s32 luaResult = luaL_loadbuffer(vm, (const_cstr)buffer.addr, buffer.length, bufferName);
        if (luaResult != 0)
        {
            LOG_ERROR("Error compiling script: %s", lua_tostring(vm, -1));
            compileSucceed = false;
            lua_pop(vm, 1);
            MessageBeep(MB_ICONERROR);
            break;
        }

        lua_pop(vm, 1);
    }
    scratch_end(&scratch);

    if (compileSucceed)
    {
        lua_close(s_context.vm);
        allocator_reset(&s_context.vmAllocator);
        s_context.vm = nullptr;
        LOG_DEBUG("VM thread unloaded");

        SCRLoadVMThread(tstr_literal(LITERAL("data")));
        MessageBeep(MB_OK);
        return true;
    }
    else
    {
        return false;
    }
}

void SCRCleanUp()
{
    LOG_SCOPE(scripting);
    FLORAL_ASSERT(s_context.ready);
    if (!s_context.ready)
    {
        return;
    }

    lua_close(s_context.vm);
    s_context.ready = false;
    LOG_DEBUG("Scripting context destroyed.");
}

void SCRAddEntryPoint(const tstr& i_filePath)
{
    dll_t<tstr>::node_t* node = arena_push_pod(&s_context.arena, dll_t<tstr>::node_t);
    node->data = tstr_duplicate(&s_context.arena, i_filePath);
    dll_push_back(&s_context.entryPoints, node);
}

void SCRRunFile(const tstr& i_filePath)
{
    LOG_DEBUG(LITERAL("Run script: %s"), i_filePath.data);
    arena_t* const arena = &s_context.arena;
    lua_State* const vm = s_context.vm;

    scratch_region_t scratch = scratch_begin(arena);

    file_handle_t fh = file_ropen(&s_context.scriptFileGroup, i_filePath);
    const_buffer_t buffer = file_read_all(fh, scratch.arena);
    file_close(&fh);

    c8* bufferName = arena_push_podarr(scratch.arena, c8, FLORAL_MAX_NAME_LENGTH);
    to_cstr(i_filePath.data, bufferName, FLORAL_MAX_NAME_LENGTH);

    SCRStackGuard guard(vm);
    s32 luaResult = luaL_loadbuffer(vm, (const_cstr)buffer.addr, buffer.length, bufferName);
    if (luaResult != 0)
    {
        LOG_ERROR("Error compiling script: %s", lua_tostring(vm, -1));
    }
    else
    {
        luaResult = lua_pcall(vm, 0, LUA_MULTRET, 0);
        FLORAL_ASSERT_MSG(luaResult == 0, lua_tostring(vm, -1));
    }

    scratch_end(&scratch);
}

void SCRRegisterFunc(lua_CFunction i_func, const_cstr i_exportedFunc, voidptr i_lightUserData)
{
    lua_State* const vm = s_context.vm;

    SCRStackGuard guard(vm);
    lua_pushlightuserdata(vm, i_lightUserData);
    lua_pushlightuserdata(vm, &s_context);
    lua_pushcclosure(vm, i_func, 2);
    lua_setglobal(vm, i_exportedFunc);
}
