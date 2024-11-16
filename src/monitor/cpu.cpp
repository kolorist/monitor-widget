#include "cpu.h"

#include <Pdh.h>
#include <Windows.h>
#include <intrin.h>

#include <floral/container.h>
#include <floral/memory.h>
#include <floral/string_utils.h>
#include <floral/thread_context.h>

#include "cpu_intel.h"
#include "cpu_amd.h"

namespace cpu
{

struct PerfCounter
{
    u32 nameHash;
    PDH_HCOUNTER handle;
};

enum class Vendor : u8
{
    Undefined = 0,
    Intel,
    AMD
};

struct State
{
    // performance counter reader (same with what are displayed in Performance Monitor application)
    dll_t<PerfCounter> pcs;
    PDH_HQUERY pcQuery;

    str8 vendorId;
    Vendor vendor;
    u8 family;
    u8 model;
    u8 stepping;

    s32 numProcessors;
    u32 pageSize;

    // vendor dispatch table
    void (*readProcessorTemperature)(f32* o_packageTemp, f32* o_coreTemps, u32* i_coreIds, u32 i_numCores);

    arena_t arena;
};

static State* s_state = nullptr;

PerfCounter* GetOrAddPerfCounter(const tstr& i_counterName)
{
    const u32 pcNameHash = tstr_fnv1a32_hash(i_counterName);
    dll_t<PerfCounter>::node_t* it = nullptr;
    dll_for_each(&s_state->pcs, it)
    {
        if (it->data.nameHash == pcNameHash)
        {
            return &it->data;
        }
    }

    dll_t<PerfCounter>::node_t* newPc = arena_create_dll_node(&s_state->arena, PerfCounter);
    newPc->data.nameHash = pcNameHash;
    dll_push_back(&s_state->pcs, newPc);
    if (s_state->pcQuery != INVALID_HANDLE_VALUE)
    {
        if (PdhAddEnglishCounter(s_state->pcQuery, i_counterName.data, NULL, &newPc->data.handle) != ERROR_SUCCESS)
        {
            newPc->data.handle = INVALID_HANDLE_VALUE;
        }
    }

    return &newPc->data;
}

bool Initialize(linear_allocator_t* i_allocator)
{
    arena_t arena = create_arena(i_allocator, SIZE_KB(16));
    s_state = arena_push_pod(&arena, State);
    s_state->arena = arena;

    // query basic system information
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    s_state->numProcessors = 0;
    for (u32 i = 0; i < systemInfo.dwNumberOfProcessors; i++)
    {
        s32 processorMask = 1 << i;
        if (systemInfo.dwActiveProcessorMask & processorMask)
        {
            s_state->numProcessors++;
        }
    }
    s_state->pageSize = systemInfo.dwPageSize;

    // get cpu vendor string. Ref: Open-Source Register Reference For AMD Family Processors
    s32 registers[4];
    c8 vendorId[13];
    __cpuid(registers, 0);
    memcpy(&vendorId[0], (cstr)&registers[1], 4);
    memcpy(&vendorId[4], (cstr)&registers[3], 4);
    memcpy(&vendorId[8], (cstr)&registers[2], 4);
    vendorId[12] = 0;
    s_state->vendorId = str8_duplicate(&s_state->arena, vendorId);
    if (str8_compare(s_state->vendorId, str8_literal("GenuineIntel")) == 0)
    {
        s_state->vendor = Vendor::Intel;
    }
    else if (str8_compare(s_state->vendorId, str8_literal("AuthenticAMD")) == 0)
    {
        s_state->vendor = Vendor::AMD;
    }
    else
    {
        s_state->vendor = Vendor::Undefined;
    }

    // get cpu identifiers. Ref: Open-Source Register Reference For AMD Family Processors
    __cpuid(registers, 1);
    s_state->family = ((registers[0] & 0x0FF00000) >> 20) + ((registers[0] & 0x0F00) >> 8);
    s_state->model = ((registers[0] & 0x0F0000) >> 12) + ((registers[0] & 0xF0) >> 4);
    s_state->stepping = (registers[0] & 0x0F);

    // u16 logicalProcessorsCount = (registers[1] & 0x07FF0000) >> 16;

    // TODO: L1 cache info + properties
    // TODO: L2 cache info + properties
    // TODO: L3 cache info + properties

    // Windows' performance counters
    s_state->pcs = create_dll<PerfCounter>();
    if (PdhOpenQuery(NULL, NULL, &s_state->pcQuery) != ERROR_SUCCESS)
    {
        s_state->pcQuery = INVALID_HANDLE_VALUE;
    }

    // dispatch table
    switch (s_state->vendor)
    {
    case Vendor::Intel:
    {
        IntelInitialize(i_allocator);
        s_state->readProcessorTemperature = &IntelReadProcessorTemperature;
        break;
    }

    case Vendor::AMD:
    {
        s_state->readProcessorTemperature = &AMDReadProcessorTemperature;
        break;
    }

    default:
        break;
    }

    return true;
}

void UpdateOSPerfCounters()
{
    if (s_state->pcQuery != INVALID_HANDLE_VALUE)
    {
        PdhCollectQueryData(s_state->pcQuery);
    }
}

void ReadProcessorUtilization(f32* o_avgLoad, f32* o_coreLoads, u32* i_coreIds, u32 i_numCores)
{
    if (o_avgLoad)
    {
        PerfCounter* counter = GetOrAddPerfCounter(tstr_literal(LITERAL("\\Processor(_Total)\\% Processor Time")));
        PDH_FMT_COUNTERVALUE counterVal;
        PdhGetFormattedCounterValue(counter->handle, PDH_FMT_DOUBLE, NULL, &counterVal);
        *o_avgLoad = (f32)counterVal.doubleValue;
    }

    if (o_coreLoads && i_coreIds && i_numCores > 0)
    {
        scratch_region_t scratch = thread_scratch_begin();
        for (u32 i = 0; i < i_numCores; i++)
        {
            tstr perfCounterName = tstr_printf(scratch.arena, LITERAL("\\Processor(%d)\\%% Processor Time"), i);
            PerfCounter* counter = GetOrAddPerfCounter(perfCounterName);
            PDH_FMT_COUNTERVALUE counterVal;
            PdhGetFormattedCounterValue(counter->handle, PDH_FMT_DOUBLE, NULL, &counterVal);
            o_coreLoads[i] = (f32)counterVal.doubleValue;
        }
        thread_scratch_end(&scratch);
    }
}

void ReadMemoryUtilization(s32* o_physical, s32* o_virtual)
{
    MEMORYSTATUSEX memStatus = {};
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memStatus);
    if (o_physical)
    {
        *o_physical = (s32)memStatus.dwMemoryLoad;
    }
    if (o_virtual)
    {
        size virtualMemUsed = memStatus.ullTotalPageFile - memStatus.ullAvailPageFile;
        f64 virtualMemLoad = (f64)virtualMemUsed * 100.0 / (f64)memStatus.ullTotalPageFile;
        *o_virtual = (s32)virtualMemLoad;
    }
}

void ReadProcessorTemperature(f32* o_packageTemp, f32* o_coreTemps, u32* i_coreIds, u32 i_numCores)
{
    return s_state->readProcessorTemperature(o_packageTemp, o_coreTemps, i_coreIds, i_numCores);
}

} // namespace cpu
