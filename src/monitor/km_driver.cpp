#include "km_driver.h"

#include <floral/log.h>
#include <floral/memory.h>
#include <floral/string_utils.h>
#include <floral/thread_context.h>

namespace kmdrv
{

State* g_state = nullptr;

// We want to use rdmsr instruction but it is not available at Ring 3 (User Priviledge)
// but only in Ring 0 (Kernel).
// And in order to gain access to Ring 0, we need a kernel driver. WinRing0 can help us.
// It is hosted at: https://github.com/GermanAizek/WinRing0.git
// Normally, we should try to build everything ourself but we can't do here, because
// if we compile WinRing0, we won't be enable to run it anywhere as Windows normally prevents
// running unsigned drivers.
// Fortunately for us, we have a prebuilt signed kernel lying around 'WinRing0x64.sys'.
// And We will use that.
// OLS stands for OpenLibSys.org.
static constexpr const_tcstr const k_driverId = LITERAL("WinRing0_1_2_0"); // WinRing0/WinRing0Dll/OlsIoctl.h
static constexpr const_tcstr const k_driverHandlePath = LITERAL("\\\\.\\WinRing0_1_2_0");
static constexpr const_tcstr const k_driverInstallPath = LITERAL("data/WinRing0x64.sys");
static constexpr u32 k_DeviceType = 40000; // WinRing0/WinRing0Sys/OpenLibSys.c

constexpr u32 BuildIOCTLCode(u32 deviceType, u32 function, u32 method, u32 access)
{
    return (deviceType << 16) | (access << 14) | (function << 2) | method;
}

enum class IOCTLMethod : u8
{
    Buffered = 0,
    InDirect = 1,
    OutDirect = 2,
    Neither = 3
};

enum class IOCTLAccess : u8
{
    Any = 0,
    Read = 1,
    Write = 2
};

// ref: WinRing0/WinRing0Dll/OlsIoctl.h
enum class IOCTLFunctionCode : u16
{
    GetDriverVersion = 0x800,
    GetRefCount = 0x801,
    ReadPCIConfig = 0x851,
    WritePCIConfig = 0x852,
    ReadMSR = 0x821,
};

// ref: WinRing0/WinRing0Dll/OlsIoctl.h
enum class IOCTLCode : u32
{
    GetDriverVersion = BuildIOCTLCode(k_DeviceType, (u32)IOCTLFunctionCode::GetDriverVersion, (u32)IOCTLMethod::Buffered, (u32)IOCTLAccess::Any),
    GetRefCount = BuildIOCTLCode(k_DeviceType, (u32)IOCTLFunctionCode::GetRefCount, (u32)IOCTLMethod::Buffered, (u32)IOCTLAccess::Any),
    ReadPCIConfig = BuildIOCTLCode(k_DeviceType, (u32)IOCTLFunctionCode::ReadPCIConfig, (u32)IOCTLMethod::Buffered, (u32)IOCTLAccess::Read),
    WritePCIConfig = BuildIOCTLCode(k_DeviceType, (u32)IOCTLFunctionCode::WritePCIConfig, (u32)IOCTLMethod::Buffered, (u32)IOCTLAccess::Write),
    ReadMSR = BuildIOCTLCode(k_DeviceType, (u32)IOCTLFunctionCode::ReadMSR, (u32)IOCTLMethod::Buffered, (u32)IOCTLAccess::Any),
};

bool Initialize(linear_allocator_t* i_allocator)
{
    LOG_SCOPE(kmdrv);

    arena_t arena = create_arena(i_allocator, SIZE_KB(16));
    g_state = arena_push_pod(&arena, State);
    g_state->arena = arena;

    SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scManager == NULL)
    {
        LOG_ERROR("Cannot establish connection to Service Control Manager. Are you running with Admin permision?");
        return false;
    }

    // Open driver, stop and remove old driver if needed
    HANDLE device = CreateFile(k_driverHandlePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (device == INVALID_HANDLE_VALUE)
    {
        scratch_region_t scratch = thread_scratch_begin();
        LOG_WARNING("Cannot open driver. Error code: %d", GetLastError());
        LOG_DEBUG("Stop and uninstalling old driver...");
        SC_HANDLE service = OpenService(scManager, k_driverId, SERVICE_ALL_ACCESS);
        if (service != NULL)
        {
            SERVICE_STATUS serviceStatus;
            if (!ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus)) // stop the driver service
            {
                LOG_WARNING("Cannot stop driver service");
            }
            DeleteService(service); // delete the old driver service
            CloseServiceHandle(service);
            LOG_DEBUG("Stopped and uninstalled old driver service");
        }
        else
        {
            LOG_WARNING("Cannot open driver service. Perhaps the driver was never installed.");
        }

        LOG_DEBUG("Installing new driver service...");
        TCHAR* fullInstallPath = arena_push_podarr(scratch.arena, TCHAR, 2048);
        GetFullPathName(k_driverInstallPath, 2048, fullInstallPath, NULL);
        service = CreateService(scManager, k_driverId, k_driverId, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
                                SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, fullInstallPath, NULL, NULL, NULL, NULL, NULL);
        if (service == NULL)
        {
            DWORD error = GetLastError();
            if (error == ERROR_SERVICE_EXISTS)
            {
                LOG_WARNING("Driver already exists");
            }
        }
        else
        {
            LOG_DEBUG("Driver installed");
            CloseServiceHandle(service);
        }

        LOG_DEBUG("Starting driver service...");
        service = OpenService(scManager, k_driverId, SERVICE_ALL_ACCESS);
        if (service != NULL)
        {
            if (StartService(service, 0, NULL) != NO_ERROR)
            {
                if (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)
                {
                    LOG_WARNING("Driver already started");
                }
            }
            else
            {
                LOG_INFO("Driver started");
            }

            CloseServiceHandle(service);
        }
        else
        {
            LOG_ERROR("Cannot start driver");
            return false;
        }

        // reopen the driver
        device = CreateFile(k_driverHandlePath, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (device == INVALID_HANDLE_VALUE)
        {
            LOG_ERROR("Cannot create Device handle. Error code: %d", GetLastError());
            return false;
        }

        scratch_end(&scratch);
    }
    FLORAL_ASSERT(device != INVALID_HANDLE_VALUE);

    HANDLE pciMutex = CreateMutex(NULL, FALSE, TEXT("Global\\Access_PCI"));
    if (pciMutex == NULL)
    {
        pciMutex = OpenMutex(SYNCHRONIZE, FALSE, TEXT("Global\\Access_PCI"));
    }
    if (pciMutex == NULL)
    {
        LOG_ERROR("Cannot create PCI mutex.");
        return false;
    }

    g_state->device = device;
    g_state->pciMutex = pciMutex;

    if (scManager != NULL)
    {
        CloseServiceHandle(scManager);
    }

    u32 driverVersion = 0;
    u32 refCount = 0;
    DeviceIoControl(g_state->device, (u32)IOCTLCode::GetDriverVersion, NULL, 0, &driverVersion, sizeof(u32), NULL, NULL);
    DeviceIoControl(g_state->device, (u32)IOCTLCode::GetRefCount, NULL, 0, &refCount, sizeof(u32), NULL, NULL);
    LOG_DEBUG("Driver version: %d. Active driver users count: %d", driverVersion, refCount);

    return true;
}

bool BeginPCI()
{
    return g_state->device != INVALID_HANDLE_VALUE &&
           WaitForSingleObject(g_state->pciMutex, 10) == WAIT_OBJECT_0;
}

void EndPCI()
{
    ReleaseMutex(g_state->pciMutex);
}

u32 ReadSMN(u32 i_address)
{
#pragma pack(1)
    struct PCIConfigDwordWriteInput
    {
        u32 pciAddress;
        u32 regAddress;
        u32 value;
    };
    struct PCIConfigDwordReadInput
    {
        u32 pciAddress;
        u32 regAddress;
    };
#pragma pack()

    PCIConfigDwordWriteInput write = {
        .pciAddress = 0,
        .regAddress = 0x60,
        .value = i_address
    };

    PCIConfigDwordReadInput read = {
        .pciAddress = 0,
        .regAddress = 0x64
    };
    DeviceIoControl(g_state->device, (u32)IOCTLCode::WritePCIConfig, &write, sizeof(write), NULL, 0, NULL, NULL);
    u32 value = 0;
    DWORD byteReturned = 0;
    DeviceIoControl(g_state->device, (u32)IOCTLCode::ReadPCIConfig, &read, sizeof(read), &value, sizeof(u32), &byteReturned, NULL);
    FLORAL_ASSERT(byteReturned == sizeof(u32));
    return value;
}

u64 ReadMSR(u32 i_address)
{
    u64 value = 0;
    DWORD byteReturned = 0;
    DeviceIoControl(g_state->device, (u32)IOCTLCode::ReadMSR, &i_address, sizeof(u32), &value, sizeof(u64), &byteReturned, NULL);
    FLORAL_ASSERT(byteReturned == sizeof(u64));
    return value;
}

} // namespace kmdrv
