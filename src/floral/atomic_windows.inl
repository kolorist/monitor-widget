#include <Windows.h>

///////////////////////////////////////////////////////////////////////////////

u32 interlocked_exchange(ATOMIC_TYPE(u32) * io_target, const u32 i_value)
{
    return InterlockedExchange(io_target, i_value);
}

u32 interlocked_decrement(ATOMIC_TYPE(u32) * io_target)
{
    return InterlockedDecrement(io_target);
}

u32 interlocked_compare_exchange(ATOMIC_TYPE(u32) * io_target, const u32 i_exchange, const u32 i_comperand)
{
    return InterlockedCompareExchange(io_target, i_exchange, i_comperand);
}
