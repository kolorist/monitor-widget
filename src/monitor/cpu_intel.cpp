#include "cpu_intel.h"

#include "km_driver.h"

#include <floral/memory.h>

namespace cpu
{

struct IntelState
{
    s32 ptccTemp;

    arena_t arena;
};

static IntelState* s_intelState = nullptr;

// ref: Intel 64 and IA-32 Architectures Software Developer's Manual - Volume 4
constexpr u32 k_MSR_TEMPERATURE_TARGET = 0x1a2;
constexpr u32 k_IA32_PACKAGE_THERM_STATUS = 0x1b1;

void IntelInitialize(linear_allocator_t* i_allocator)
{
    arena_t arena = create_arena(i_allocator, SIZE_KB(16));
    s_intelState = arena_push_pod(&arena, IntelState);
    s_intelState->arena = arena;

    const u64 value = kmdrv::ReadMSR(k_MSR_TEMPERATURE_TARGET);
    const s32 tempTarget = s32((value >> 16) & 0x7f);         // in degrees
    const s32 tccActivationOffset = s32((value >> 24) & 0xf); // in degrees
    s_intelState->ptccTemp = tempTarget + tccActivationOffset;
}

void IntelReadProcessorTemperature(f32* o_packageTemp, f32* o_coreTemps, u32* i_coreIds, u32 i_numCores)
{
    // ref: Intel 64 and IA-32 Architectures Software Developer's Manual - Volume 3 - section 16.9
    const u64 value = kmdrv::ReadMSR(k_IA32_PACKAGE_THERM_STATUS);
    const s32 digitalReadout = s32((value >> 16) & 0x7f); // in degrees
    const s32 pkgTemp = s_intelState->ptccTemp - digitalReadout;

    *o_packageTemp = (f32)pkgTemp;
}

} // namespace cpu
