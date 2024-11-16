#include "system_info.h"

#include "assert.h"
#include "log.h"

#if defined(FLORAL_PLATFORM_WINDOWS)
#  include <Windows.h>
#endif

// ----------------------------------------------------------------------------

static platform_info_t get_platform_info()
{
    platform_info_t info = {};

#if defined(FLORAL_PLATFORM_WINDOWS)
    u8 scratch[2048];

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    info.pageSize = sysInfo.dwPageSize;

    DWORD bufferSize = 0;
    GetLogicalProcessorInformation(NULL, &bufferSize);
    FLORAL_ASSERT(bufferSize <= 2048);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)scratch;
    GetLogicalProcessorInformation(buffer, &bufferSize);
    u32 itemCount = bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

    for (u32 i = 0; i < itemCount; i++)
    {
        if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1 && buffer[i].Cache.Type == CacheData)
        {
            info.l1CacheSize = buffer[i].Cache.Size;
            info.l1CacheLineSize = buffer[i].Cache.LineSize;
            info.l1Associativity = buffer[i].Cache.Associativity;
            break;
        }
    }
#endif

    return info;
}

static platform_info_t s_platformInfo = get_platform_info();

// ----------------------------------------------------------------------------

void si_dump()
{
    LOG_SCOPE(si);

    LOG_DEBUG("Platform memory information:");
    LOG_DEBUG("- Malloc's alignment: %zd bytes", MEMORY_DEFAULT_MALLOC_ALIGNMENT);
    LOG_DEBUG("- Data alignment: %zd bytes", MEMORY_DEFAULT_ALIGNMENT);
    LOG_DEBUG("- SIMD alignment: %zd bytes", MEMORY_SIMD_ALIGNMENT);
    LOG_DEBUG("- Page size: %zd bytes", s_platformInfo.pageSize);
    LOG_DEBUG("- Layer 1 Data cache size: %zd bytes", s_platformInfo.l1CacheSize);
    LOG_DEBUG("- Layer 1 Data cache line size: %zd bytes", s_platformInfo.l1CacheLineSize);
    LOG_DEBUG("- Layer 1 Data cache associativity: %zd", s_platformInfo.l1Associativity);
}

