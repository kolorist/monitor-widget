#include <Windows.h>

#include "log.h"
#include "misc.h"

///////////////////////////////////////////////////////////////////////////////

struct platform_file_t
{
    HANDLE handle;
    tstr path;
};

///////////////////////////////////////////////////////////////////////////////

tstr path_get_working_directory(arena_t* const i_arena)
{
    size requiredLength = GetCurrentDirectory(0, NULL);
    tchar* buffer = arena_push_podarr(i_arena, tchar, requiredLength);
    GetCurrentDirectory((DWORD)requiredLength, buffer);

    return tstr_literal(buffer);
}

///////////////////////////////////////////////////////////////////////////////

voidptr platform_arena_push_platform_file(arena_t* const i_arena, const tstr& i_baseDir, const tstr& i_subPath)
{
    platform_file_t* platformFile = arena_push_pod(i_arena, platform_file_t);

    // manual string magic
    size length = i_baseDir.length + i_subPath.length;
    tchar* buffer = arena_push_podarr(i_arena, tchar, length + 2);
    mem_copy(buffer, i_baseDir.data, i_baseDir.length * sizeof(tchar));
    buffer[i_baseDir.length] = LITERAL('\\');
    mem_copy(buffer + i_baseDir.length + 1, i_subPath.data, i_subPath.length * sizeof(tchar));
    buffer[length + 1] = 0;
    for (size i = 0; i < length; i++)
    {
        if (buffer[i] == LITERAL('/'))
        {
            buffer[i] = LITERAL('\\');
        }
    }

    platformFile->handle = INVALID_HANDLE_VALUE;
    platformFile->path = { .data = buffer, .length = length };
    return platformFile;
}

void platform_initialize_file_group(file_system_t* i_fileSystem, file_group_t* const io_fileGroup, const tstr& i_subPath)
{
    scratch_region_t scratch = scratch_begin(&i_fileSystem->arena);
    arena_t* const arena = &io_fileGroup->arena;

    if (i_subPath.length > 0)
    {
        tstr subPath = tstr_replace_char(scratch.arena, i_subPath, LITERAL('/'), LITERAL('\\'));
        io_fileGroup->baseDir = tstr_printf(arena, LITERAL("%s\\%s"), i_fileSystem->workingDirectory.data, subPath.data);
    }
    else
    {
        io_fileGroup->baseDir = tstr_duplicate(arena, i_fileSystem->workingDirectory);
    }

    scratch_end(&scratch);
}

size platform_find_all_files_internal(file_system_t* const i_fileSystem, const tstr& i_subPath, const tstr& i_ext, const tstr& i_remap, dll_t<file_t>* o_fileList, arena_t* const i_arena)
{
    size fileCount = 0;
    scratch_region_t scratch = scratch_begin(&i_fileSystem->arena);
    tstr absPathFiltered = tstr_printf(scratch.arena, LITERAL("%s\\%s\\*.%s"),
                                       i_fileSystem->workingDirectory.data, i_subPath.data, i_ext.data);
    WIN32_FIND_DATA* findData = arena_push_pod(scratch.arena, WIN32_FIND_DATA);
    HANDLE hFind = FindFirstFile(absPathFiltered.data, findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                dll_t<file_t>::node_t* fileNode = arena_push_pod(i_arena, dll_t<file_t>::node_t);
                tstr remapPath;
                if (i_remap.length > 0)
                {
                    remapPath = tstr_printf(i_arena, LITERAL("%s/%s"), i_remap.data, findData->cFileName);
                }
                else
                {
                    remapPath = tstr_duplicate(i_arena, findData->cFileName);
                }

                tstr platformPath = path_join(scratch.arena, i_subPath, tstr_literal(findData->cFileName));
                fileNode->data.path = remapPath;
                fileNode->data.pathHash = tstr_crc32_hash(remapPath);
                voidptr platform = platform_arena_push_platform_file(i_arena, i_fileSystem->workingDirectory, platformPath);
                fileNode->data.platform = platform;

                dll_push_back(o_fileList, fileNode);
                fileCount++;
            }
        } while (FindNextFile(hFind, findData));

        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    // recurse subdirectories
    tstr absPathNoFiltered = tstr_printf(scratch.arena, LITERAL("%s\\%s\\*"),
                                         i_fileSystem->workingDirectory.data, i_subPath.data);
    hFind = FindFirstFile(absPathNoFiltered.data, findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        while (FindNextFile(hFind, findData))
        {
            if ((findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                tcstr_compare(findData->cFileName, LITERAL(".")) != 0 &&
                tcstr_compare(findData->cFileName, LITERAL("..")) != 0)
            {
                tstr fileName = tstr_literal(findData->cFileName);

                tstr subPath = path_join(scratch.arena, i_subPath, fileName);
                tstr remapPath = path_join(scratch.arena, i_remap, fileName);
                fileCount += platform_find_all_files_internal(i_fileSystem, subPath, i_ext, remapPath, o_fileList, i_arena);
            }
        }
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    scratch_end(&scratch);
    return fileCount;
}

void platform_find_all_files(file_system_t* i_fileSystem, file_group_t* const io_fileGroup, const tstr& i_subPath, const tstr& i_ext, const tstr& i_remap)
{
    scratch_region_t scratch = scratch_begin(&i_fileSystem->arena);
    arena_t* const arena = &io_fileGroup->arena;

    if (i_subPath.length > 0)
    {
        tstr subPath = tstr_replace_char(scratch.arena, i_subPath, '/', '\\');
        io_fileGroup->baseDir = tstr_printf(arena, LITERAL("%s\\%s"), i_fileSystem->workingDirectory.data, subPath.data);
    }
    else
    {
        io_fileGroup->baseDir = tstr_duplicate(arena, i_fileSystem->workingDirectory);
    }
    io_fileGroup->fileCount = platform_find_all_files_internal(i_fileSystem, i_subPath, i_ext, i_remap, &io_fileGroup->fileList, arena);

    scratch_end(&scratch);
}

error_code_e platform_file_ropen(voidptr io_platformFile)
{
    platform_file_t* const pf = (platform_file_t*)io_platformFile;
    pf->handle = CreateFile(pf->path.data, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
    if (pf->handle == INVALID_HANDLE_VALUE)
    {
        FLORAL_ASSERT(false);
        return error_code_e::failed_to_open_file;
    }
    return error_code_e::success;
}

error_code_e platform_file_wopen(voidptr io_platformFile)
{
    platform_file_t* const pf = (platform_file_t*)io_platformFile;
    pf->handle = CreateFile(pf->path.data, GENERIC_WRITE, FILE_SHARE_WRITE || FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (pf->handle == INVALID_HANDLE_VALUE)
    {
        return error_code_e::failed_to_open_file;
    }
    return error_code_e::success;
}

size platform_file_get_size(voidptr i_platformFile)
{
    platform_file_t* const pf = (platform_file_t*)i_platformFile;
    LARGE_INTEGER fSize;
    GetFileSizeEx(pf->handle, &fSize);
    return (size)fSize.QuadPart;
}

void platform_file_read(voidptr i_platformFile, voidptr io_buffer, const size i_bufferSize)
{
    platform_file_t* const pf = (platform_file_t*)i_platformFile;
    DWORD bytesRead = 0;
    BOOL status = ReadFile(pf->handle, io_buffer, (DWORD)i_bufferSize, &bytesRead, NULL);
    FLORAL_ASSERT(status == TRUE);
}

void platform_file_write(voidptr i_platformFile, const_voidptr i_buffer, const size i_bufferSize)
{
    platform_file_t* const pf = (platform_file_t*)i_platformFile;
    DWORD byteWritten = 0;
    WriteFile(pf->handle, i_buffer, (DWORD)i_bufferSize, &byteWritten, NULL);
}

void platform_file_flush(voidptr i_platformFile)
{
    platform_file_t* const pf = (platform_file_t*)i_platformFile;
    FlushFileBuffers(pf->handle);
}

void platform_file_close(voidptr i_platformFile)
{
    platform_file_t* const pf = (platform_file_t*)i_platformFile;
    BOOL success = CloseHandle(pf->handle);
    pf->handle = INVALID_HANDLE_VALUE;
    FLORAL_ASSERT(success);
}

void platform_make_directories(const tstr& i_baseDir, const tstr& i_subDir, arena_t* const i_arena)
{
    scratch_region_t scratch = scratch_begin(i_arena);
    tstr subDir = tstr_replace_char(scratch.arena, i_subDir, '/', '\\');
    tstr absDir = tstr_printf(scratch.arena, LITERAL("%s\\%s"), i_baseDir.data, subDir.data);
    for (size i = 0; i < absDir.length; i++)
    {
        if (absDir.data[i] == LITERAL('\\'))
        {
            tstr subPath = tstr_duplicate(scratch.arena, absDir.data, i);
            if (!CreateDirectory(subPath.data, NULL))
            {
                DWORD error = GetLastError();
                if (error != ERROR_ALREADY_EXISTS)
                {
                    break;
                }
            }
        }
    }
    CreateDirectory(absDir.data, NULL);
    scratch_end(&scratch);
}

void debug_platform_dump_file_group(file_group_t* i_fileGroup)
{
    dll_t<file_t>::node_t* it = nullptr;
    LOG_DEBUG(LITERAL("Base directory: %s"), i_fileGroup->baseDir.data);
    LOG_DEBUG(LITERAL("File count: %zd"), i_fileGroup->fileCount);
    size idx = 1;
    dll_for_each(&i_fileGroup->fileList, it)
    {
        const platform_file_t* const platform = (const platform_file_t*)it->data.platform;
        LOG_DEBUG(LITERAL("#%d: %s"), idx, it->data.path.data);
        LOG_DEBUG(LITERAL("=> %s"), platform->path.data);
        idx++;
    }
}
