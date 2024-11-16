#pragma once

#include "floral/stdaliases.h"

// this order of includes is important
#include <ws2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <tchar.h>

namespace network
{
// ----------------------------------------------------------------------------

struct Info
{
    u64 sentBytes;
    u64 receivedBytes;
    f32 receivedBytesPerSec;
    f32 sentBytesPerSec;
};


bool Initialize();
void CleanUp();
void ReadStats(f32* o_ingress, f32* o_egress, u64* o_sent, u64* o_received);

// ----------------------------------------------------------------------------
} // namespace network
