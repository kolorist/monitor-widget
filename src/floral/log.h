#pragma once
#include "stdaliases.h"
#include "string_utils.h"
#include "memory.h"
#include "file_system.h"

///////////////////////////////////////////////////////////////////////////////

enum class log_level_e : u8
{
    verbose = 0,
    debug,
    info,
    warning,
    error
};

typedef void (*log_cstr_t)(voidptr i_ctx, log_level_e i_logLevel, const_cstr i_msg, const size i_msgLen, arena_t* const i_arena);
typedef void (*log_wcstr_t)(voidptr i_ctx, log_level_e i_logLevel, const_wcstr i_msg, const size i_msgLen, arena_t* const i_arena);

struct log_scope_t
{
    str8 ansiName;
    str16 unicodeName;
    aptr origin;
};

struct log_scope_helper_t
{
    log_scope_helper_t(const_cstr i_name);
    ~log_scope_helper_t();

    log_scope_t scope;
};

struct logger_entry_t
{
    voidptr context;
    log_cstr_t logCstr;
    log_wcstr_t logWcstr;
};

struct log_context_t
{
    u64 frameIndex;
    log_scope_t* scopes;
    u32 scopeIdx;
    log_level_e logLevel;

    logger_entry_t* loggers;
    u32 loggerCount;

    str8 ansiName;
    str16 unicodeName;
    arena_t arena;
};

log_context_t create_log_context(const_cstr i_name, log_level_e i_logLevel, linear_allocator_t* const i_allocator);
void log_set_context(log_context_t* const i_logCtx);
log_context_t* log_get_context();

void log_context_add_logger(log_context_t* const i_logCtx, log_cstr_t i_logCstr, log_wcstr_t i_logWcstr, voidptr i_loggerCtx);
void log_tick();
void log_message(log_level_e i_logLevel, const_cstr i_fmt, ...);
void log_message(log_level_e i_logLevel, const_wcstr i_fmt, ...);
log_scope_t log_push_scope(const_cstr i_name);
void log_pop_scope(const log_scope_t* const i_scope);

///////////////////////////////////////////////////////////////////////////////

struct console_logger_t
{
    mutex_t mutex;
    log_level_e level;
};

console_logger_t create_console_logger(log_level_e i_logLevel);
console_logger_t* log_context_create_console_logger(log_context_t* const i_logCtx, log_level_e i_logLevel);
void console_logger_log_message_cstr(voidptr i_ctx, log_level_e i_logLevel, const_cstr i_msg, const size i_msgLen, arena_t* const i_arena);
void console_logger_log_message_wcstr(voidptr i_ctx, log_level_e i_logLevel, const_wcstr i_msg, const size i_msgLen, arena_t* const i_arena);

///////////////////////////////////////////////////////////////////////////////

struct windows_logger_t
{
    mutex_t mutex;
    log_level_e level;
};

windows_logger_t create_windows_logger(log_level_e i_logLevel);
windows_logger_t* log_context_create_windows_logger(log_context_t* const i_logCtx, log_level_e i_logLevel);
void windows_logger_log_message_cstr(voidptr i_ctx, log_level_e i_logLevel, const_cstr i_msg, const size i_msgLen, arena_t* const i_arena);
void windows_logger_log_message_wcstr(voidptr i_ctx, log_level_e i_logLevel, const_wcstr i_msg, const size i_msgLen, arena_t* const i_arena);

///////////////////////////////////////////////////////////////////////////////

struct file_logger_t
{
    mutex_t mutex;
    file_handle_t logFile;
    log_level_e level;
};

file_logger_t create_file_logger(log_level_e i_logLevel, file_group_t* const i_fileGroup, tstr i_filePath);
file_logger_t* log_context_create_file_logger(log_context_t* const i_logCtx, log_level_e i_logLevel);
void file_logger_log_message_cstr(voidptr i_ctx, log_level_e i_logLevel, const_cstr i_msg, const size i_msgLen, arena_t* const i_arena);
void file_logger_log_message_wcstr(voidptr i_ctx, log_level_e i_logLevel, const_wcstr i_msg, const size i_msgLen, arena_t* const i_arena);

///////////////////////////////////////////////////////////////////////////////

#define LOG_SCOPE(name)     log_scope_helper_t scope##name(#name)

#define LOG_VERBOSE(...)    log_message(log_level_e::verbose, __VA_ARGS__)
#define LOG_DEBUG(...)      log_message(log_level_e::debug, __VA_ARGS__)
#define LOG_INFO(...)       log_message(log_level_e::info, __VA_ARGS__)
#define LOG_WARNING(...)    log_message(log_level_e::warning, __VA_ARGS__)
#define LOG_ERROR(...)      log_message(log_level_e::error, __VA_ARGS__)
