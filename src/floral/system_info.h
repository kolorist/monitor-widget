#pragma once

#include "stdaliases.h"

// ----------------------------------------------------------------------------

struct platform_info_t
{
    size pageSize;

    // data cache
    u32 l1CacheSize;
    u16 l1CacheLineSize;
    u8 l1Associativity;
};

void si_dump();
