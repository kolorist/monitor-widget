#pragma once

#include <floral/stdaliases.h>

struct linear_allocator_t;

namespace gpu
{

void NVInitialize(linear_allocator_t* i_allocator);
void NVReadUtilization(u32* o_geLoad, u32* o_fbLoad, u32* o_vidLoad, u32* o_busLoad);
void NVReadVRAMUtilization(u32* o_memLoad);
void NVReadTemperature(f32* o_temp);

} // namespace gpu
