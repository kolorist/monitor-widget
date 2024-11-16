#pragma once

#include <floral/assert.h>
#include <floral/log.h>
#include <floral/memory.h>
#include <floral/stdaliases.h>
#include <floral/thread.h>
#include <floral/container.h>
#include <floral/atomic.h>

extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

struct FTContext;

// ----------------------------------------------------------------------------

enum class SCRErrorCode : u8
{
    Success = 0,

    ErrorCannotOpenScriptFile,
    ErrorTypeMismatch,
};

struct SCRContext
{
    file_system_t* fileSystem;
    file_group_t scriptFileGroup;
    lua_State* vm;
    freelist_allocator_t vmAllocator;

    dll_t<tstr> entryPoints;

    arena_t arena;
    bool ready;
};

struct SCRClosure
{
    s32 returnValuesCount;
};

struct SCRStackGuard
{
    SCRStackGuard(lua_State* const i_vm)
        : vm(i_vm)
    {
        initialStackTop = lua_gettop(vm);
    }

    ~SCRStackGuard()
    {
        s32 currTop = lua_gettop(vm);
        FLORAL_ASSERT_MSG(initialStackTop == currTop, "Stack is corrupted!");
    }

    lua_State* vm;
    s32 initialStackTop;
};

// ----------------------------------------------------------------------------

void SCRInitialize(file_system_t* const i_fs, linear_allocator_t* const i_allocator);
SCRContext* SCRGetContext();
void SCRLoadVMThread(const tstr& i_subPath);
bool SCRReloadVMThread();
void SCRCleanUp();
void SCRAddEntryPoint(const tstr& i_filePath);
void SCRRegisterFunc(lua_CFunction i_func, const_cstr i_exportedFunc, voidptr i_lightUserData);
template <typename CallContext>
void SCRCallFunc(const_cstr i_funcName, CallContext* const i_callCtx);

// ----------------------------------------------------------------------------

template <typename CallContext>
void SCRCallFunc(const_cstr i_funcName, CallContext* const i_callCtx)
{
    LOG_SCOPE(scripting);
    SCRContext* context = SCRGetContext();

    SCRStackGuard guard(context->vm);
    lua_getfield(context->vm, LUA_GLOBALSINDEX, i_funcName);
    s32 numArgs = i_callCtx->PushArgs(context->vm);
    if (lua_pcall(context->vm, numArgs, i_callCtx->returnValuesCount, 0) != 0)
    {
        LOG_ERROR("Error calling function '%s': %s", i_funcName, lua_tostring(context->vm, -1));
        FLORAL_DEBUG_BREAK();
        return;
    }
    i_callCtx->DeserializeReturnValues(context->vm);
}
