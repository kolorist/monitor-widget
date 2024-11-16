#include "network.h"

#include <floral/log.h>
#include <floral/misc.h>
#include <floral/time.h>

namespace network
{
// ----------------------------------------------------------------------------

struct State
{
    u64 receivedBytes;
    u64 sentBytes;
    f64 updateTimepoint;

    NET_LUID interfaceLuid;

    bool ready;
};

struct State s_state;

// ----------------------------------------------------------------------------

bool Initialize()
{
    LOG_SCOPE(network);

    s_state.ready = false;
    s_state.interfaceLuid.Value = 0;

    MIB_IF_TABLE2* ifTable = nullptr;
    DWORD ret = GetIfTable2(&ifTable);
    if (ret == NO_ERROR)
    {
        LOG_DEBUG("Available network interface:");
        for (size i = 0; i < ifTable->NumEntries; i++)
        {
            const MIB_IF_ROW2& row = ifTable->Table[i];
            if (row.MediaConnectState == MediaConnectStateConnected)
            {
                if (row.InterfaceAndOperStatusFlags.HardwareInterface == TRUE && s_state.interfaceLuid.Value == 0)
                {
                    s_state.interfaceLuid = row.InterfaceLuid;
                    LOG_DEBUG(TEXT("(*) %s"), row.Alias);
                }
                else
                {
                    LOG_DEBUG(TEXT("    %s"), row.Alias);
                }
            }
        }

        FreeMibTable(ifTable);

        s_state.ready = true;
        LOG_DEBUG("Network Driver initialized.");
        return true;
    }
    else
    {
        LOG_ERROR("Error when getting network's IfTable.");
        return false;
    }
}

void CleanUp()
{
    if (s_state.ready)
    {
        s_state.ready = false;
        LOG_VERBOSE("Network Driver destroyed.");
        return;
    }
}

void ReadStats(f32* o_ingress, f32* o_egress, u64* o_sent, u64* o_received)
{
    MIB_IF_ROW2 row = {};
    row.InterfaceLuid = s_state.interfaceLuid;
    if (s_state.ready && GetIfEntry2(&row) == NO_ERROR)
    {
        u64 receivedBytes = row.InOctets;
        u64 sentBytes = row.OutOctets;
        if (s_state.receivedBytes > 0 && s_state.sentBytes > 0)
        {
            f64 deltaTime = (time_get_absolute_highres_ms() - s_state.updateTimepoint) * 0.001f;
            u64 deltaReceivedBytes = receivedBytes - s_state.receivedBytes;
            u64 deltaSentBytes = sentBytes - s_state.sentBytes;

            if (o_egress)
            {
                *o_egress = (f32)((f64)deltaSentBytes / deltaTime);
            }
            if (o_ingress)
            {
                *o_ingress = (f32)((f64)deltaReceivedBytes / deltaTime);
            }
        }
        s_state.receivedBytes = receivedBytes;
        s_state.sentBytes = sentBytes;

        if (o_received)
        {
            *o_received = receivedBytes;
        }
        if (o_sent)
        {
            *o_sent = sentBytes;
        }
    }

    s_state.updateTimepoint = time_get_absolute_highres_ms();
}

// ----------------------------------------------------------------------------
} // namespace network
