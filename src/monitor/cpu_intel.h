#pragma once

#include <floral/stdaliases.h>

struct linear_allocator_t;

namespace cpu
{

void IntelInitialize(linear_allocator_t* i_allocator);
void IntelReadProcessorTemperature(f32* o_packageTemp, f32* o_coreTemps, u32* i_coreIds, u32 i_numCores);

} // namespace cpu
