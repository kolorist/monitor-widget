#include "gpu.h"

#include <floral/memory.h>
#include <floral/log.h>

#include "gpu_nvidia.h"

namespace gpu
{

struct State
{
    arena_t arena;
};

static State* s_state = nullptr;

void Initialize(linear_allocator_t* i_allocator)
{
    arena_t arena = create_arena(i_allocator, SIZE_KB(16));
    s_state = arena_push_pod(&arena, State);
    s_state->arena = arena;

    NVInitialize(i_allocator);
}

void ReadUtilization(u32* o_geLoad, u32* o_fbLoad, u32* o_vidLoad, u32* o_busLoad)
{
    NVReadUtilization(o_geLoad, o_fbLoad, o_vidLoad, o_busLoad);
}

void ReadVRAMUtilization(u32* o_memLoad)
{
    NVReadVRAMUtilization(o_memLoad);
}

void ReadTemperature(f32* o_temp)
{
    NVReadTemperature(o_temp);
}

} // namespace gpu
