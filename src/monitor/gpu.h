#pragma once

#include <floral/stdaliases.h>

struct linear_allocator_t;

namespace gpu
{
void Initialize(linear_allocator_t* i_allocator);

void ReadUtilization(u32* o_geLoad, u32* o_fbLoad, u32* o_vidLoad, u32* o_busLoad);
void ReadVRAMUtilization(u32* o_memLoad);
void ReadTemperature(f32* o_temp);
} // namespace gpu
