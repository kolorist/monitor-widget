#pragma once

#include <floral/stdaliases.h>

struct linear_allocator_t;

namespace cpu
{

bool Initialize(linear_allocator_t* i_allocator);
void UpdateOSPerfCounters();

void ReadProcessorUtilization(f32* o_avgLoad, f32* o_coreLoads, u32* i_coreIds, u32 i_numCores);
void ReadMemoryUtilization(s32* o_physical, s32* o_virtual);
void ReadProcessorTemperature(f32* o_packageTemp, f32* o_coreTemps, u32* i_coreIds, u32 i_numCores);

} // namespace cpu
