#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if defined(FLORAL_PLATFORM_WINDOWS)
#  include <Windows.h>
#elif defined(FLORAL_PLATFORM_ANDROID)
#  include <android/log.h>
#else
// TODO
#endif

#include "assert.h"
#include "misc.h"
#include "thread.h"
#include "thread_context.h"
#include "time.h"

///////////////////////////////////////////////////////////////////////////////

static mutex_t s_platformLogMtx = create_mutex();
thread_local log_context_t* s_tlLogContext = nullptr;

void log_set_context(log_context_t* const i_logCtx)
{
    s_tlLogContext = i_logCtx;
}

log_context_t* log_get_context()
{
    if (s_tlLogContext == nullptr)
    {
        thread_context_t* threadContext = thread_get_context();
        log_context_t* logContext = (log_context_t*)allocator_alloc(&threadContext->allocator, sizeof(log_context_t));
        *logContext = create_log_context(nullptr, log_level_e::verbose, &threadContext->allocator);
#if defined(FLORAL_PLATFORM_WINDOWS)
        console_logger_t* consoleLogger = log_context_create_console_logger(logContext, log_level_e::verbose);
        windows_logger_t* windowsLogger = log_context_create_windows_logger(logContext, log_level_e::verbose);
        log_context_add_logger(logContext, &console_logger_log_message_cstr, &console_logger_log_message_wcstr, consoleLogger);
        log_context_add_logger(logContext, &windows_logger_log_message_cstr, &windows_logger_log_message_wcstr, windowsLogger);
#elif defined(FLORAL_PLATFORM_ANDROID)
        // TODO: add more platforms
#endif
        log_set_context(logContext);
        LOG_WARNING("A temporary log context and probably also a thread context have been created. You should not ignore this in release build!");
    }
    return s_tlLogContext;
}

log_scope_helper_t::log_scope_helper_t(const_cstr i_name)
{
    scope = log_push_scope(i_name);
}

log_scope_helper_t::~log_scope_helper_t()
{
    log_pop_scope(&scope);
}

log_context_t create_log_context(const_cstr i_name, log_level_e i_logLevel, linear_allocator_t* const i_allocator)
{
    arena_t arena = create_arena(i_allocator, SIZE_KB(24));

    log_context_t logCtx;
    logCtx.frameIndex = 0;
    logCtx.scopes = arena_push_podarr(&arena, log_scope_t, LOG_MAX_SCOPES);
    logCtx.scopeIdx = 0;
    logCtx.logLevel = i_logLevel;

    logCtx.loggers = arena_push_podarr(&arena, logger_entry_t, LOG_MAX_LOGGERS);
    logCtx.loggerCount = 0;

    if (i_name)
    {
        logCtx.ansiName = str8_duplicate(&arena, i_name);
    }
    else
    {
        logCtx.ansiName = thread_get_id_as_str(&arena);
    }
    logCtx.unicodeName = str16_duplicate(&arena, logCtx.ansiName);
    logCtx.arena = arena;
    return logCtx;
}

void log_context_add_logger(log_context_t* const i_logCtx, log_cstr_t i_logCstr, log_wcstr_t i_logWcstr, voidptr i_loggerCtx)
{
    i_logCtx->loggers[i_logCtx->loggerCount].context = i_loggerCtx;
    i_logCtx->loggers[i_logCtx->loggerCount].logCstr = i_logCstr;
    i_logCtx->loggers[i_logCtx->loggerCount].logWcstr = i_logWcstr;
    i_logCtx->loggerCount++;
}

void log_tick()
{
    log_context_t* const logCtx = log_get_context();
    logCtx->frameIndex++;
}

void log_message(log_level_e i_logLevel, const_cstr i_fmt, ...)
{
    log_context_t* const logCtx = log_get_context();
    arena_t* const arena = &logCtx->arena;

    if (i_logLevel < logCtx->logLevel)
    {
        return;
    }

    scratch_region_t scratch = scratch_begin(arena);
    const s32 maxLogLength = 4096;

    cstr buffer = arena_push_podarr(scratch.arena, c8, maxLogLength);
    cstr pBuff = buffer;
    cstr pBuffEnd = buffer + maxLogLength;

    const u32 fidx = logCtx->frameIndex % 1000;
    const timepoint tp = time_get_local_now();
    static const_cstr k_mappings[] = {
        "[%02d-%02d-%4d %02d:%02d:%02d.%03d] [%s] [%03d] [verbose] ", // log_level_e::verbose
        "[%02d-%02d-%4d %02d:%02d:%02d.%03d] [%s] [%03d] [debug  ] ", // log_level_e::debug
        "[%02d-%02d-%4d %02d:%02d:%02d.%03d] [%s] [%03d] [info   ] ", // log_level_e::info
        "[%02d-%02d-%4d %02d:%02d:%02d.%03d] [%s] [%03d] [warning] ", // log_level_e::warning
        "[%02d-%02d-%4d %02d:%02d:%02d.%03d] [%s] [%03d] [error  ] ", // log_level_e::error
    };
    pBuff += cstr_snprintf(pBuff, pBuffEnd - pBuff, k_mappings[(u8)i_logLevel],
                           tp.day, tp.month, tp.year, tp.hour, tp.minute, tp.second, tp.millisecond,
                           logCtx->ansiName.data, fidx);

    if (logCtx->scopeIdx == 0)
    {
        mem_copy(pBuff, "(/) ", 4 * sizeof(c8));
        pBuff += 4;
    }
    else
    {
        log_scope_t* const scopes = logCtx->scopes;
        *(pBuff++) = '(';
        for (u32 i = 0; i < logCtx->scopeIdx; i++)
        {
            pBuff += cstr_snprintf(pBuff, pBuffEnd - pBuff, "/%s", scopes[i].ansiName.data);
        }
        *(pBuff++) = ')';
        *(pBuff++) = ' ';
    }

    va_list args; // NOLINT(cppcoreguidelines-init-variables)
    va_start(args, i_fmt);
    pBuff += cstr_vsnprintf(pBuff, pBuffEnd - pBuff, i_fmt, args);
    FLORAL_ASSERT(pBuffEnd - pBuff > 0);

    logger_entry_t* loggers = logCtx->loggers;
    const size validLength = pBuff - buffer;
    for (u32 i = 0; i < logCtx->loggerCount; i++)
    {
        loggers[i].logCstr(loggers[i].context, i_logLevel, buffer, validLength, scratch.arena);
    }

    va_end(args);
    scratch_end(&scratch);
}

void log_message(log_level_e i_logLevel, const_wcstr i_fmt, ...)
{
    log_context_t* const logCtx = log_get_context();
    arena_t* const arena = &logCtx->arena;

    if (i_logLevel < logCtx->logLevel)
    {
        return;
    }

    scratch_region_t scratch = scratch_begin(arena);
    const s32 maxLogLength = 4096;

    wcstr buffer = arena_push_podarr(scratch.arena, c16, maxLogLength);
    wcstr pBuff = buffer;
    wcstr pBuffEnd = buffer + maxLogLength;
    const u32 fidx = logCtx->frameIndex % 1000;
    const timepoint tp = time_get_local_now();
    static const_wcstr k_mappings[] = {
        L"[%02d-%02d-%4d %02d:%02d:%02d.%03d] [%s] [%03d] [verbose] ", // log_level_e::verbose
        L"[%02d-%02d-%4d %02d:%02d:%02d.%03d] [%s] [%03d] [debug  ] ", // log_level_e::debug
        L"[%02d-%02d-%4d %02d:%02d:%02d.%03d] [%s] [%03d] [info   ] ", // log_level_e::info
        L"[%02d-%02d-%4d %02d:%02d:%02d.%03d] [%s] [%03d] [warning] ", // log_level_e::warning
        L"[%02d-%02d-%4d %02d:%02d:%02d.%03d] [%s] [%03d] [error  ] ", // log_level_e::error
    };
    pBuff += wcstr_snprintf(pBuff, pBuffEnd - pBuff, k_mappings[(u8)i_logLevel],
                            tp.day, tp.month, tp.year, tp.hour, tp.minute, tp.second, tp.millisecond,
                            logCtx->unicodeName.data, fidx);

    if (logCtx->scopeIdx == 0)
    {
        mem_copy(pBuff, L"(/) ", 4 * sizeof(c16));
        pBuff += 4;
    }
    else
    {
        log_scope_t* const scopes = logCtx->scopes;
        *(pBuff++) = L'(';
        for (u32 i = 0; i < logCtx->scopeIdx; i++)
        {
            pBuff += wcstr_snprintf(pBuff, pBuffEnd - pBuff, L"/%s", scopes[i].unicodeName.data);
        }
        *(pBuff++) = L')';
        *(pBuff++) = L' ';
    }

    va_list args; // NOLINT(cppcoreguidelines-init-variables)
    va_start(args, i_fmt);
    pBuff += wcstr_vsnprintf(pBuff, pBuffEnd - pBuff, i_fmt, args);
    FLORAL_ASSERT(pBuffEnd - pBuff > 0);

    logger_entry_t* loggers = logCtx->loggers;
    const size validLength = pBuff - buffer;
    for (u32 i = 0; i < logCtx->loggerCount; i++)
    {
        loggers[i].logWcstr(loggers[i].context, i_logLevel, buffer, validLength, scratch.arena);
    }

    va_end(args);
    scratch_end(&scratch);
}

log_scope_t log_push_scope(const_cstr i_name)
{
    log_context_t* const logCtx = log_get_context();
    arena_t* const arena = &logCtx->arena;

    aptr origin = arena_tellp(arena);
    log_scope_t scope = {
        .ansiName = str8_duplicate(arena, i_name),
        .unicodeName = str16_duplicate(arena, i_name),
        .origin = origin
    };
    logCtx->scopes[logCtx->scopeIdx] = scope;
    logCtx->scopeIdx++;
    FLORAL_ASSERT(logCtx->scopeIdx <= LOG_MAX_SCOPES);
    return scope;
}

void log_pop_scope(const log_scope_t* const i_scope)
{
    log_context_t* const logCtx = log_get_context();
    arena_t* const arena = &logCtx->arena;

    arena_pop_to(arena, i_scope->origin);
    logCtx->scopeIdx--;
    FLORAL_ASSERT(logCtx->scopeIdx >= 0);
}

///////////////////////////////////////////////////////////////////////////////

console_logger_t create_console_logger(log_level_e i_logLevel)
{
    return {
        .mutex = create_mutex(),
        .level = i_logLevel
    };
}

console_logger_t* log_context_create_console_logger(log_context_t* const i_logCtx, log_level_e i_logLevel)
{
    console_logger_t* logger = arena_push_pod(&i_logCtx->arena, console_logger_t);
    logger->mutex = create_mutex();
    logger->level = i_logLevel;
    return logger;
}

void console_logger_log_message_cstr(voidptr i_ctx, log_level_e i_logLevel, const_cstr i_msg, const size i_msgLen, arena_t* const i_arena)
{
    console_logger_t* const logger = (console_logger_t*)i_ctx;

    if ((u8)i_logLevel >= (u8)logger->level)
    {
        scratch_region_t scratch = scratch_begin(i_arena);
        const size bufferCapacity = i_msgLen + 32;
        cstr buffer = arena_push_podarr(scratch.arena, c8, bufferCapacity);
        s32 length = cstr_snprintf(buffer, bufferCapacity, "[console] %s\r\n", i_msg);

        lock_guard_t guard(&logger->mutex);
#if defined(FLORAL_PLATFORM_WINDOWS)
        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), buffer, (DWORD)length, NULL, NULL);
#else
#endif
        scratch_end(&scratch);
    }
}

void console_logger_log_message_wcstr(voidptr i_ctx, log_level_e i_logLevel, const_wcstr i_msg, const size i_msgLen, arena_t* const i_arena)
{
    console_logger_t* const logger = (console_logger_t*)i_ctx;

    if ((u8)i_logLevel >= (u8)logger->level)
    {
        scratch_region_t scratch = scratch_begin(i_arena);
        const size bufferCapacity = i_msgLen + 32;
        wcstr buffer = arena_push_podarr(scratch.arena, c16, bufferCapacity);
        s32 length = wcstr_snprintf(buffer, bufferCapacity, L"[console] %s\r\n", i_msg);

        lock_guard_t guard(&logger->mutex);
#if defined(FLORAL_PLATFORM_WINDOWS)
        WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), buffer, (DWORD)length, NULL, NULL);
#else
#endif
        scratch_end(&scratch);
    }
}

///////////////////////////////////////////////////////////////////////////////

windows_logger_t create_windows_logger(log_level_e i_logLevel)
{
    return {
        .mutex = create_mutex(),
        .level = i_logLevel
    };
}

windows_logger_t* log_context_create_windows_logger(log_context_t* const i_logCtx, log_level_e i_logLevel)
{
    windows_logger_t* logger = arena_push_pod(&i_logCtx->arena, windows_logger_t);
    logger->mutex = create_mutex();
    logger->level = i_logLevel;
    return logger;
}

void windows_logger_log_message_cstr(voidptr i_ctx, log_level_e i_logLevel, const_cstr i_msg, const size i_msgLen, arena_t* const i_arena)
{
    windows_logger_t* const logger = (windows_logger_t*)i_ctx;

    if ((u8)i_logLevel >= (u8)logger->level)
    {
        scratch_region_t scratch = scratch_begin(i_arena);
        const size bufferCapacity = i_msgLen + 32;
        cstr buffer = arena_push_podarr(scratch.arena, c8, bufferCapacity);
        cstr_snprintf(buffer, bufferCapacity, "%s\r\n", i_msg);

        lock_guard_t guard(&logger->mutex);
        OutputDebugStringA(buffer);
        scratch_end(&scratch);
    }
}

void windows_logger_log_message_wcstr(voidptr i_ctx, log_level_e i_logLevel, const_wcstr i_msg, const size i_msgLen, arena_t* const i_arena)
{
    windows_logger_t* const logger = (windows_logger_t*)i_ctx;

    if ((u8)i_logLevel >= (u8)logger->level)
    {
        scratch_region_t scratch = scratch_begin(i_arena);
        const size bufferCapacity = i_msgLen + 32;
        wcstr buffer = arena_push_podarr(scratch.arena, c16, bufferCapacity);
        wcstr_snprintf(buffer, bufferCapacity, L"%s\r\n", i_msg);

        lock_guard_t guard(&logger->mutex);
        OutputDebugStringW(buffer);
        scratch_end(&scratch);
    }
}

///////////////////////////////////////////////////////////////////////////////

file_logger_t create_file_logger(log_level_e i_logLevel, file_group_t* const i_fileGroup, tstr i_filePath)
{
    return {
        .mutex = create_mutex(),
        .logFile = file_wopen(i_fileGroup, i_filePath),
        .level = i_logLevel
    };
}

file_logger_t* log_context_create_file_logger(log_context_t* const i_logCtx, log_level_e i_logLevel, file_group_t* const i_fileGroup, tstr i_filePath)
{
    file_logger_t* logger = arena_push_pod(&i_logCtx->arena, file_logger_t);
    logger->mutex = create_mutex();
    logger->logFile = file_wopen(i_fileGroup, i_filePath),
    logger->level = i_logLevel;
    return logger;
}

void file_logger_log_message_cstr(voidptr i_ctx, log_level_e i_logLevel, const_cstr i_msg, const size i_msgLen, arena_t* const i_arena)
{
    file_logger_t* const logger = (file_logger_t*)i_ctx;

    if ((u8)i_logLevel >= (u8)logger->level)
    {
        scratch_region_t scratch = scratch_begin(i_arena);
        const size bufferCapacity = i_msgLen + 32;
        cstr buffer = arena_push_podarr(scratch.arena, c8, bufferCapacity);
        s32 bufferLength = cstr_snprintf(buffer, bufferCapacity, "%s\n", i_msg);

        lock_guard_t guard(&logger->mutex);
        file_write(logger->logFile, buffer, bufferLength);
        file_flush(logger->logFile);

        scratch_end(&scratch);
    }
}

void file_logger_log_message_wcstr(voidptr i_ctx, log_level_e i_logLevel, const_wcstr i_msg, const size i_msgLen, arena_t* const i_arena)
{
    MARK_UNUSED(i_ctx);
    MARK_UNUSED(i_logLevel);
    MARK_UNUSED(i_msg);
    MARK_UNUSED(i_msgLen);
    MARK_UNUSED(i_arena);
}
