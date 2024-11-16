#include "file_system.h"

#include "error.h"

///////////////////////////////////////////////////////////////////////////////

tstr path_join(arena_t* const i_arena, const tstr& i_a, const tstr& i_b)
{
    if (i_a.length > 0)
    {
        return tstr_printf(i_arena, LITERAL("%s/%s"), i_a.data, i_b.data);
    }
    else
    {
        return i_b;
    }
}

///////////////////////////////////////////////////////////////////////////////

voidptr platform_arena_push_platform_file(arena_t* const i_arena, const tstr& i_baseDir, const tstr& i_subPath);
void platform_initialize_file_group(file_system_t* i_fileSystem, file_group_t* const io_fileGroup, const tstr& i_subPath);
void platform_find_all_files(file_system_t* i_fileSystem, file_group_t* const io_fileGroup, const tstr& i_subPath, const tstr& i_ext, const tstr& i_remap);
error_code_e platform_file_ropen(voidptr io_platformFile);
error_code_e platform_file_wopen(voidptr io_platformFile);
size platform_file_get_size(voidptr i_platformFile);
void platform_file_read(voidptr i_platformFile, voidptr io_buffer, const size i_bufferSize);
void platform_file_write(voidptr i_platformFile, const_voidptr i_buffer, const size i_bufferSize);
void platform_file_flush(voidptr i_platformFile);
void platform_file_close(voidptr i_platformFile);
void platform_make_directories(const tstr& i_baseDir, const tstr& i_subDir, arena_t* const i_arena);
void debug_platform_dump_file_group(file_group_t* i_fileGroup);

///////////////////////////////////////////////////////////////////////////////

file_system_t create_file_system(linear_allocator_t* const i_allocator)
{
    file_system_t fileSystem;
    fileSystem.allocator = i_allocator;
    fileSystem.arena = create_arena(i_allocator, SIZE_KB(256));
    fileSystem.workingDirectory = path_get_working_directory(&fileSystem.arena);
    return fileSystem;
}

file_group_t create_file_group(file_system_t* const i_fileSystem, const tstr& i_path)
{
    file_group_t fileGroup;
    fileGroup.fileList = create_dll<file_t>();
    fileGroup.arena = create_arena(i_fileSystem->allocator, SIZE_MB(1));
    fileGroup.fileCount = 0;
    platform_initialize_file_group(i_fileSystem, &fileGroup, i_path); // fills in baseDir
    platform_make_directories(fileGroup.baseDir, tstr_literal(LITERAL("")), &fileGroup.arena);
    return fileGroup;
}

file_group_t create_file_group(file_system_t* const i_fileSystem)
{
    file_group_t fileGroup;
    fileGroup.fileList = create_dll<file_t>();
    fileGroup.arena = create_arena(i_fileSystem->allocator, SIZE_MB(1));
    fileGroup.fileCount = 0;
    return fileGroup;
}

void file_group_reset(file_group_t* const io_fileGroup)
{
    dll_reset(&io_fileGroup->fileList);
    arena_reset(&io_fileGroup->arena);
}

void file_system_set_working_directory(file_system_t* const i_fileSystem, const tstr& i_path)
{
    i_fileSystem->workingDirectory = tstr_duplicate(&i_fileSystem->arena, i_path);
}

file_group_t file_system_find_all_files(file_system_t* const i_fileSystem, const tstr& i_subPath, const tstr& i_ext)
{
    return file_system_find_all_files(i_fileSystem, i_subPath, i_ext, tstr_literal(LITERAL("")));
}

file_group_t file_system_find_all_files(file_system_t* const i_fileSystem, const tstr& i_subPath, const tstr& i_ext, const tstr& i_remap)
{
    file_group_t fileGroup;
    fileGroup.fileList = create_dll<file_t>();
    fileGroup.arena = create_arena(i_fileSystem->allocator, SIZE_MB(1));
    platform_find_all_files(i_fileSystem, &fileGroup, i_subPath, i_ext, i_remap); // fills in baseDir, fileList and fileCount
    return fileGroup;
}

void file_system_find_all_files(file_system_t* const i_fileSystem, const tstr& i_subPath,
                                const tstr& i_ext, file_group_t* const o_fileGroup)
{
    file_system_find_all_files(i_fileSystem, i_subPath, i_ext, tstr_literal(LITERAL("")), o_fileGroup);
}

void file_system_find_all_files(file_system_t* const i_fileSystem, const tstr& i_subPath,
                                const tstr& i_ext, const tstr& i_remap, file_group_t* const o_fileGroup)
{
    platform_find_all_files(i_fileSystem, o_fileGroup, i_subPath, i_ext, i_remap); // fills in baseDir, fileList and fileCount
}

file_handle_t file_ropen(file_group_t* const i_fileGroup, const tstr& i_path)
{
    const u32 pathHash = tstr_crc32_hash(i_path);
    dll_t<file_t>::node_t* fileNode = nullptr;
    dll_for_each(&i_fileGroup->fileList, fileNode)
    {
        if (fileNode->data.pathHash == pathHash)
        {
            voidptr platformFile = fileNode->data.platform;
            error_code_e errCode = platform_file_ropen(platformFile);
            return { .platform = platformFile, .hasErrors = (errCode != error_code_e::success) };
        }
    }
    return { .platform = nullptr, .hasErrors = true };
}

size file_get_size(const file_handle_t& i_handle)
{
    FLORAL_ASSERT(!i_handle.hasErrors);
    return platform_file_get_size(i_handle.platform);
}

const_buffer_t file_read_all(const file_handle_t& i_handle, arena_t* const i_arena)
{
    size requiredSize = file_get_size(i_handle);
    voidptr buffer = arena_push(i_arena, requiredSize);
    platform_file_read(i_handle.platform, buffer, requiredSize);
    return { .addr = buffer, .length = requiredSize };
}

void file_read(const file_handle_t& i_handle, const size i_bufferSize, voidptr io_buffer)
{
    platform_file_read(i_handle.platform, io_buffer, i_bufferSize);
}

void file_close(file_handle_t* const i_handle)
{
    platform_file_close(i_handle->platform);
    i_handle->hasErrors = false;
}

file_handle_t file_wopen(file_group_t* const i_fileGroup, const tstr& i_path)
{
    arena_t* const arena = &i_fileGroup->arena;
    dll_t<file_t>* const fileList = &i_fileGroup->fileList;

    const u32 pathHash = tstr_crc32_hash(i_path);
    voidptr platform = nullptr;
    dll_t<file_t>::node_t* it = nullptr;
    dll_for_each(fileList, it)
    {
        if (it->data.pathHash == pathHash)
        {
            platform = it->data.platform;
            break;
        }
    }

    if (!platform)
    {
        // temporary use the area as a scratch region to make the directories tree to store the file
        scratch_region_t scratch = scratch_begin(arena);
        // TODO: get base dir should be a dedicated function?
        for (ssize i = (ssize)i_path.length; i >= 0; i--)
        {
            if (i_path.data[i] == LITERAL('/'))
            {
                tstr relBaseDir = tstr_duplicate(scratch.arena, i_path.data, i);
                platform_make_directories(i_fileGroup->baseDir, relBaseDir, scratch.arena);
                break;
            }
        }
        scratch_end(&scratch);

        dll_t<file_t>::node_t* fileNode = arena_push_pod(arena, dll_t<file_t>::node_t);
        fileNode->data.path = tstr_duplicate(arena, i_path);
        fileNode->data.pathHash = pathHash;
        platform = platform_arena_push_platform_file(arena, i_fileGroup->baseDir, i_path);
        fileNode->data.platform = platform;

        dll_push_back(fileList, fileNode);
        i_fileGroup->fileCount++;
    }
    FLORAL_ASSERT(platform != nullptr);
    error_code_e errCode = platform_file_wopen(platform);
    return { .platform = platform, .hasErrors = (errCode != error_code_e::success) };
}

void file_write_all(const file_handle_t& i_handle, const buffer_t* const i_buffer)
{
    // TODO: reset file
    platform_file_write(i_handle.platform, i_buffer->addr, i_buffer->length);
}

void file_write(const file_handle_t& i_handle, const_voidptr i_buffer, const size i_bufferSize)
{
    platform_file_write(i_handle.platform, i_buffer, i_bufferSize);
}

void file_flush(const file_handle_t& i_handle)
{
    platform_file_flush(i_handle.platform);
}

bool file_exist(file_group_t* const i_fileGroup, const tstr& i_path)
{
    dll_t<file_t>* const fileList = &i_fileGroup->fileList;
    const u32 pathHash = tstr_crc32_hash(i_path);
    dll_t<file_t>::node_t* it = nullptr;
    dll_for_each(fileList, it)
    {
        if (it->data.pathHash == pathHash)
        {
            return true;
        }
    }
    return false;
}

void debug_dump_file_group(file_group_t* const i_fileGroup)
{
    debug_platform_dump_file_group(i_fileGroup);
}

#if defined(FLORAL_PLATFORM_WINDOWS)
#  include "file_system_windows.inl"
#endif
