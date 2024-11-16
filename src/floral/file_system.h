#pragma once

#include "container.h"
#include "memory.h"
#include "pods.h"
#include "stdaliases.h"
#include "string_utils.h"

///////////////////////////////////////////////////////////////////////////////

tstr path_get_working_directory(arena_t* const i_arena);
tstr path_join(arena_t* const i_arena, const tstr& i_a, const tstr& i_b);

///////////////////////////////////////////////////////////////////////////////

struct file_system_t
{
    tstr workingDirectory; // platform-dependant path style

    linear_allocator_t* allocator;
    arena_t arena;
};

struct file_t
{
    tstr path;
    u32 pathHash;
    voidptr platform;
};

struct file_group_t
{
    size fileCount;
    tstr baseDir; // platform-dependant path style
    dll_t<file_t> fileList;

    arena_t arena;
};

struct file_handle_t
{
    voidptr platform;
    bool hasErrors;
};

file_system_t create_file_system(linear_allocator_t* const i_allocator);
// NOTE: `i_path` is platform-dependant path style
file_group_t create_file_group(file_system_t* const i_fileSystem, const tstr& i_path);
file_group_t create_file_group(file_system_t* const i_fileSystem);
void file_group_reset(file_group_t* const io_fileGroup);
// NOTE: `i_path` is platform-dependant path style
void file_system_set_working_directory(file_system_t* const i_fileSystem, const tstr& i_path);
// NOTE: `i_subPath` and `i_ext` is platform-dependant path style
file_group_t file_system_find_all_files(file_system_t* const i_fileSystem, const tstr& i_subPath, const tstr& i_ext);
file_group_t file_system_find_all_files(file_system_t* const i_fileSystem, const tstr& i_subPath, const tstr& i_ext, const tstr& i_remap);
void file_system_find_all_files(file_system_t* const i_fileSystem, const tstr& i_subPath,
                                const tstr& i_ext, file_group_t* const o_fileGroup);
void file_system_find_all_files(file_system_t* const i_fileSystem, const tstr& i_subPath,
                                const tstr& i_ext, const tstr& i_remap, file_group_t* const o_fileGroup);

file_handle_t file_ropen(file_group_t* const i_fileGroup, const tstr& i_path);
size file_get_size(const file_handle_t& i_handle);
const_buffer_t file_read_all(const file_handle_t& i_handle, arena_t* const i_arena);
void file_read(const file_handle_t& i_handle, const size i_bufferSize, voidptr io_buffer);
void file_close(file_handle_t* const i_handle);

file_handle_t file_wopen(file_group_t* const i_fileGroup, const tstr& i_path);
void file_write_all(const file_handle_t& i_handle, const buffer_t* const i_buffer);
void file_write(const file_handle_t& i_handle, const_voidptr i_buffer, const size i_bufferSize);
void file_flush(const file_handle_t& i_handle);

bool file_exist(file_group_t* const i_fileGroup, const tstr& i_path);

void debug_dump_file_group(file_group_t* const i_fileGroup);
