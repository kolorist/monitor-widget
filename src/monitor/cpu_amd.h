#pragma once

#include <floral/stdaliases.h>

namespace cpu
{

void AMDReadProcessorTemperature(f32* o_packageTemp, f32* o_coreTemps, u32* i_coreIds, u32 i_numCores);

} // namespace cpu
