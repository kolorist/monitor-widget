#pragma once

#include <Windows.h>

#include <floral/memory.h>

namespace kmdrv
{

struct State
{
    HANDLE device;
    HANDLE pciMutex;

    arena_t arena;
};

extern State* g_state;

bool Initialize(linear_allocator_t* i_allocator);
bool BeginPCI();
void EndPCI();
u32 ReadSMN(u32 i_address);
u64 ReadMSR(u32 i_address);

} // namespace kmdrv
