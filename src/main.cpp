#include <floral/assert.h>
#include <floral/log.h>
#include <floral/memory.h>
#include <floral/thread_context.h>
#include <floral/system_info.h>
#include <floral/file_system.h>

#include <resource.h>

#include "winapi.h"
#include "files_tracker.h"
#include "scripting.h"

#include "monitor/km_driver.h"
#include "monitor/cpu.h"
#include "monitor/gpu.h"
#include "monitor/network.h"

#include "configs.h"
#include "main_dialog.h"
#include "utils.h"

// This will enable Windows' visual style
// Please make sure to link with comctl32.lib to make the visual style enabled.
#pragma comment(linker, "\"/manifestdependency:type='Win32' "       \
                        "name='Microsoft.Windows.Common-Controls' " \
                        "version='6.0.0.0' "                        \
                        "processorArchitecture='amd64' "            \
                        "publicKeyToken='6595b64144ccf1df' "        \
                        "language='*'\"")

///////////////////////////////////////////////////////////////////////////////

#if defined(RETAIL_BUILD)
typedef BOOL(WINAPI* MiniDumpWriteDump_t)(HANDLE hProcess, DWORD dwPid, HANDLE hFile,
                                          MINIDUMP_TYPE DumpType,
                                          const PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                          const PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                          const PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

long WINAPI ExceptionHandler(_EXCEPTION_POINTERS* i_exceptionPointer)
{
    HMODULE dbgModule = pxLoadLibrary(LITERAL("dbghelp.dll"));
    if (dbgModule != NULL)
    {
        MiniDumpWriteDump_t pDump = (MiniDumpWriteDump_t)pxGetProcAddress(dbgModule, "MiniDumpWriteDump");

        HANDLE hFile = pxCreateFile(LITERAL("monitor-widget.dmp"), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        _MINIDUMP_EXCEPTION_INFORMATION exceptionInfo = {};
        exceptionInfo.ThreadId = pxGetCurrentThreadId();
        exceptionInfo.ExceptionPointers = i_exceptionPointer;
        exceptionInfo.ClientPointers = FALSE;

        pDump(pxGetCurrentProcess(), pxGetCurrentProcessId(), hFile, MiniDumpNormal, &exceptionInfo, NULL, NULL);
        pxCloseHandle(hFile);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
    MARK_UNUSED(hPrevInstance);
    MARK_UNUSED(lpCmdLine);
    MARK_UNUSED(nShowCmd);
#if RETAIL_BUILD
    pxSetUnhandledExceptionFilter(ExceptionHandler);
#endif
    HANDLE currentThread = GetCurrentThread();
    SetThreadAffinityMask(currentThread, 0x1);
    // we will dynamically handle the DPI change via WM_DPICHANGED message, by using
    // PROCESS_PER_MONITOR_DPI_AWARE, we enable this message
    pxSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    pxSetPriorityClass(pxGetCurrentProcess(), REALTIME_PRIORITY_CLASS);

    linear_allocator_t masterAllocator = create_linear_allocator("'main' master allocator", SIZE_MB(16));
    thread_context_t threadContext = {
        .allocator = create_linear_allocator(&masterAllocator, "main thread context allocator", SIZE_MB(1))
    };
    thread_set_context(&threadContext);

    log_context_t logCtx = create_log_context("main", log_level_e::verbose, &threadContext.allocator);
    log_set_context(&logCtx);

    windows_logger_t windowsLogger = create_windows_logger(log_level_e::verbose);
    log_context_add_logger(&logCtx, &windows_logger_log_message_cstr, &windows_logger_log_message_wcstr, &windowsLogger);

    LOG_SCOPE(main);
    file_system_t fileSystem = create_file_system(&masterAllocator);
    file_group_t logsFileGroup = create_file_group(&fileSystem, tstr_literal(LITERAL("logs")));
    LOG_DEBUG("File system initialized");

    file_logger_t fileLogger = create_file_logger(log_level_e::verbose, &logsFileGroup, tstr_literal(LITERAL("main.log")));
    log_context_add_logger(&logCtx, &file_logger_log_message_cstr, &file_logger_log_message_wcstr, &fileLogger);
    LOG_VERBOSE("monitor-widget v0.7-alpha");

    si_dump();

    arena_t arena = create_arena(&masterAllocator, SIZE_KB(128));

    CFGInitialize(&masterAllocator, &fileSystem);

    tstr scriptPath = tstr_literal(LITERAL("data"));
    SCRInitialize(&fileSystem, &masterAllocator);
    SCRAddEntryPoint(tstr_literal(LITERAL("main.lua")));
    SCRLoadVMThread(scriptPath);

    kmdrv::Initialize(&masterAllocator);
    cpu::Initialize(&masterAllocator);
    gpu::Initialize(&masterAllocator);
    network::Initialize();

    FTInitialize(&masterAllocator);
    tstr trackingPath = tstr_printf(&arena, LITERAL("%s\\%s"), fileSystem.workingDirectory.data, scriptPath.data);
    FTStart(trackingPath);

    if (!UIMainDialogCreate(hInstance, &masterAllocator))
    {
        UTLShowMessage(NULL, UTLSeverity::Error, LITERAL("Failed to create Main Dialog."));
        return -1;
    }
    UIMainDialogRun(); // main loop is here
    UIMainDialogCleanUp();

    FTStop();
    FTCleanUp();

    SCRCleanUp();

    LOG_DEBUG("monitor-widget terminated gracefully.");
    allocator_destroy(&masterAllocator);
    return 0;
}
