#include "gpu_nvidia.h"

#include "nvapi/nvapi.h"

#include <floral/memory.h>
#include <floral/misc.h>
#include <floral/thread.h>

namespace gpu
{

struct NVidiaState
{
    NvPhysicalGpuHandle gpus[NVAPI_MAX_PHYSICAL_GPUS];
    NvU32 gpusCount;

    mutex_t mtx;
    u32 geLoad[NVAPI_MAX_PHYSICAL_GPUS];
    u32 fbLoad[NVAPI_MAX_PHYSICAL_GPUS];
    u32 vidLoad[NVAPI_MAX_PHYSICAL_GPUS];

    arena_t arena;
};

static NVidiaState* s_nvState = nullptr;

void GpuUtilizationCallback(NvPhysicalGpuHandle i_physicalGPU, NV_GPU_CLIENT_CALLBACK_UTILIZATION_DATA_V1* i_data)
{
    MARK_UNUSED(i_physicalGPU);
    lock_guard_t guard(&s_nvState->mtx);
    NvU32 gpuIdx = (NvU32)i_data->super.pCallbackParam;
    for (u32 i = 0; i < i_data->numUtils; i++)
    {
        const NV_GPU_CLIENT_UTILIZATION_DATA_V1& utils = i_data->utils[i];
        switch (utils.utilId)
        {
        case NV_GPU_CLIENT_UTIL_DOMAIN_GRAPHICS:
            s_nvState->geLoad[gpuIdx] = utils.utilizationPercent / 100;
            break;

        case NV_GPU_CLIENT_UTIL_DOMAIN_FRAME_BUFFER:
            s_nvState->fbLoad[gpuIdx] = utils.utilizationPercent / 100;
            break;

        case NV_GPU_CLIENT_UTIL_DOMAIN_VIDEO:
            s_nvState->vidLoad[gpuIdx] = utils.utilizationPercent / 100;
            break;

        default:
            break;
        }
    }
}

void NVInitialize(linear_allocator_t* i_allocator)
{
    arena_t arena = create_arena(i_allocator, SIZE_KB(8));
    s_nvState = arena_push_pod(&arena, NVidiaState);
    s_nvState->arena = arena;

    s_nvState->mtx = create_mutex();
    s_nvState->gpusCount = 0;

    NvAPI_Status initResult = NvAPI_Initialize();
    if (initResult == NVAPI_OK)
    {
        NvAPI_EnumPhysicalGPUs(s_nvState->gpus, &s_nvState->gpusCount);

        for (NvU32 i = 0; i < s_nvState->gpusCount; i++)
        {
            NV_GPU_CLIENT_UTILIZATION_PERIODIC_CALLBACK_SETTINGS callbackSettings = {};
            callbackSettings.super.super.pCallbackParam = (void*)i;
            callbackSettings.super.callbackPeriodms = 1000;
            callbackSettings.version = NV_GPU_CLIENT_UTILIZATION_PERIODIC_CALLBACK_SETTINGS_VER;
            callbackSettings.callback = &GpuUtilizationCallback;
            NvAPI_GPU_ClientRegisterForUtilizationSampleUpdates(s_nvState->gpus[i], &callbackSettings);
        }
    }
}

void NVReadUtilization(u32* o_geLoad, u32* o_fbLoad, u32* o_vidLoad, u32* o_busLoad)
{
    u32 geLoad = 0;
    u32 fbLoad = 0;
    u32 vidLoad = 0;
    u32 busLoad = 0;
    if (s_nvState->gpusCount > 0)
    {
        lock_guard_t guard(&s_nvState->mtx);

        geLoad = s_nvState->geLoad[0];
        fbLoad = s_nvState->fbLoad[0];
        vidLoad = s_nvState->vidLoad[0];
    }

    *o_geLoad = geLoad;
    *o_fbLoad = fbLoad;
    *o_vidLoad = vidLoad;
    *o_busLoad = busLoad;
}

void NVReadVRAMUtilization(u32* o_memLoad)
{
    u32 memLoad = 0;
    if (s_nvState->gpusCount > 0)
    {
        NV_GPU_MEMORY_INFO_EX memoryInfo = {};
        memoryInfo.version = NV_GPU_MEMORY_INFO_EX_VER;
        NvAPI_GPU_GetMemoryInfoEx(s_nvState->gpus[0], &memoryInfo);
        f64 utilization = 1.0f - (f64)memoryInfo.curAvailableDedicatedVideoMemory / (f64)memoryInfo.availableDedicatedVideoMemory;
        memLoad = (u32)mathf_ceil((f32)utilization * 100.0f);
    }

    *o_memLoad = memLoad;
}

void NVReadTemperature(f32* o_temp)
{
    f32 temp = 0.0f;
    if (s_nvState->gpusCount > 0)
    {
        NV_GPU_THERMAL_SETTINGS thermalSettings = {};
        thermalSettings.version = NV_GPU_THERMAL_SETTINGS_VER;
        if (NvAPI_GPU_GetThermalSettings(s_nvState->gpus[0], NVAPI_THERMAL_TARGET_ALL, &thermalSettings) == NVAPI_OK)
        {
            for (u32 i = 0; i < NVAPI_MAX_THERMAL_SENSORS_PER_GPU; i++)
            {
                if (thermalSettings.sensor[i].target == NVAPI_THERMAL_TARGET_GPU)
                {
                    temp = (f32)thermalSettings.sensor[i].currentTemp;
                    break;
                }
            }
        }
    }

    *o_temp = temp;
}

} // namespace gpu
