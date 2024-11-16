#include "main_dialog.h"

#include <floral/log.h>
#include <floral/thread_context.h>
#include <floral/memory.h>

#include <resource.h>

#include "defines.h"
#include "configs.h"
#include "winapi.h"
#include "scripting.h"
#include "taskbar_widget.h"
#include "tray.h"
#include "utils.h"

static MainDialogState s_mainDlgState;

static void ExitApplication(s32 i_exitCode)
{
    LOG_VERBOSE("Application requested termination.");

    KillTimer(s_mainDlgState.hWnd, ID_TASKBAR_TIMER);
    {
        DWORD errorCode = GetLastError();
        LOG_WARNING("Failed to kill main dialog timer: %d", errorCode);
    }

    UIWidgetCleanUp();
    UITrayIconCleanUp();

    pxPostQuitMessage(i_exitCode);
}

static HWND TryInitializingTrayIconAndTaskBarWidget()
{
    if (!s_mainDlgState.hWnd)
    {
        LOG_WARNING("Main Dialog is not ready yet, cannot initialize Tray Icon and Widget now");
        return NULL;
    }

    bool trayIconInitialized = UITrayIconInitialize(s_mainDlgState.hInstance, s_mainDlgState.hWnd);
    if (trayIconInitialized)
    {
        HWND widgetHwnd = UIWidgetInitialize(s_mainDlgState.hInstance, &s_mainDlgState.allocator);
        if (widgetHwnd)
        {
            UIWidgetToggle(CFGGetBool(CFGKey::ShowInTaskBar));
            return widgetHwnd;
        }
        else
        {
            LOG_WARNING("Tray icon created succesfully, but failed to create taskbar widget.");
        }
    }
    else
    {
        LOG_WARNING("Failed to create tray icon.");
    }
    return NULL;
}

static void LabelSetText(HFONT i_font, HWND i_lbl, const tstr& i_str)
{
    HDC hdc = GetDC(i_lbl);
    SelectObject(hdc, i_font);
    SIZE textSize = {};
    GetTextExtentPoint32(hdc, i_str.data, (s32)i_str.length, &textSize);
    SetWindowPos(i_lbl, NULL, 0, 0, textSize.cx, textSize.cy, SWP_NOMOVE | SWP_NOZORDER);
    SetWindowText(i_lbl, i_str.data);
    ReleaseDC(i_lbl, hdc);
}

static void MainDialogUpdateDebugInfos()
{
    HFONT hFont = s_mainDlgState.hFont;

    scratch_region_t scratch = thread_scratch_begin();
    {
        allocator_t* const allocator = &SCRGetContext()->vmAllocator.base;
        f32 utilization = (f32)allocator->usedBytes / (f32)allocator->capacity;
        tstr allocsCountStr = tstr_printf(scratch.arena, LITERAL("Allocations: %d"), allocator->allocCount);
        tstr freesCountStr = tstr_printf(scratch.arena, LITERAL("Frees: %d"), allocator->freeCount);
        tstr usedMemoryStr = tstr_printf(scratch.arena, LITERAL("Used: %zd of %zd bytes (%3.2f%%)"),
                                         allocator->usedBytes, allocator->capacity, utilization);

        LabelSetText(hFont, s_mainDlgState.lblAllocsCount, allocsCountStr);
        LabelSetText(hFont, s_mainDlgState.lblFreesCount, freesCountStr);
        LabelSetText(hFont, s_mainDlgState.lblUsedMemory, usedMemoryStr);
    }
    thread_scratch_end(&scratch);
}

static INT_PTR MainDialogProc(HWND i_hwnd, UINT i_msg, WPARAM i_wParam, LPARAM i_lParam)
{
    LOG_SCOPE(dlgproc);

    s32 msgResult = 0;
    INT_PTR dlgResult = FALSE;

    switch (i_msg)
    {

    case WM_INITDIALOG:
    {
        s_mainDlgState.hFont = (HFONT)SendMessage(i_hwnd, WM_GETFONT, 0, 0);
        if (s_mainDlgState.wmTaskBarRestart == 0)
        {
            s_mainDlgState.wmTaskBarRestart = RegisterWindowMessage(LITERAL("TaskbarCreated"));
        }

        pxSetWindowLongPtr(i_hwnd, GWLP_USERDATA, (LONG_PTR)i_lParam);
        dlgResult = TRUE;
        break;
    }

    case WM_SHOWWINDOW:
    {
        if (i_wParam == TRUE)
        {
            LOG_DEBUG("Main Dialog restored");
            MainDialogUpdateDebugInfos();
            pxSetTimer(i_hwnd, ID_MAINDLG_TIMER, 1000, NULL);
        }
        else
        {
            LOG_DEBUG("Main Dialog hided");
            if (!KillTimer(i_hwnd, ID_MAINDLG_TIMER))
            {
                DWORD errorCode = GetLastError();
                LOG_WARNING("Failed to kill main dialog timer: %d", errorCode);
            }
        }

        break;
    }

    // Shell_NotifyIcon can fail when we invoke it during the time explorer.exe isn't present/ready to handle it.
    // We'll also never receive wmTaskBarRestart message if the first call to Shell_NotifyIcon failed, so we use
    // WM_WINDOWPOSCHANGING which is always received on explorer startup sequence.
    case WM_WINDOWPOSCHANGING:
    {
        LOG_SCOPE(wm_windowposchanging);
        TryInitializingTrayIconAndTaskBarWidget();
        break;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(i_wParam))
        {

        case ID_EXIT:
        {
            ExitApplication(0);
            dlgResult = TRUE;
            break;
        }

        case ID_MINIMIZE_TO_TRAY:
        {
            LOG_VERBOSE("Application minimized to tray.");
            pxShowWindow(i_hwnd, SW_HIDE);
            dlgResult = TRUE;
            break;
        }

        case ID_RESTORE_FROM_TRAY:
        {
            LOG_VERBOSE("Application restored from tray.");
            pxShowWindow(i_hwnd, SW_SHOW);
            dlgResult = TRUE;
            break;
        }

        case ID_TOGGLE_TASKBAR_INFO:
        {
            bool showInTaskBar = CFGGetBool(CFGKey::ShowInTaskBar);
            showInTaskBar = !showInTaskBar;
            if (UIWidgetToggle(showInTaskBar))
            {
                LOG_VERBOSE("Show in taskbar: %s", showInTaskBar ? "true" : "false");
                CFGSetBool(CFGKey::ShowInTaskBar, showInTaskBar);
            }
            dlgResult = TRUE;
            break;
        }

        case ID_SWITCH_MODE:
        {
            UIWidgetSwitchMode();
            dlgResult = TRUE;
            break;
        }

        case ID_START_ON_BOOT:
        {
            bool startOnBoot = CFGGetBool(CFGKey::StartOnBoot);
            OSSetStartOnBoot(!startOnBoot);
            startOnBoot = OSGetStartOnBoot();
            LOG_VERBOSE("Start with Windows: %s", startOnBoot ? "true" : "false");
            CFGSetBool(CFGKey::StartOnBoot, startOnBoot);
            break;
        }

        case ID_FORCE_CRASH:
        {
            LOG_VERBOSE("Hold on! The application is going to crash!");
            FLORAL_CRASH;
            dlgResult = TRUE;
            break;
        }

        default:
            break;
        }
        break;
    }

    case WM_CLOSE:
    {
        ExitApplication(0);
        dlgResult = TRUE;
        break;
    }

    case WM_TIMER:
    {
        switch (LOWORD(i_wParam))
        {
        case ID_MAINDLG_TIMER:
        {
            MainDialogUpdateDebugInfos();
            break;
        }

        default:
            break;
        }

        break;
    }

    case AM_TRAY:
    {
        UITrayIconHandleMessage(LOWORD(i_lParam), i_hwnd);
        dlgResult = TRUE;
        break;
    }

    default:
    {
        if (i_msg == s_mainDlgState.wmTaskBarRestart)
        {
            LOG_SCOPE(taskbar_restart);
            LOG_DEBUG("Custom message 'TaskbarCreated' received, initializing the TrayIcon...");
            TryInitializingTrayIconAndTaskBarWidget();
        }
        break;
    }
    }

    SetWindowLong(i_hwnd, DWLP_MSGRESULT, msgResult);
    return dlgResult;
}

// ----------------------------------------------------------------------------

HWND UIMainDialogCreate(HINSTANCE i_hInstance, linear_allocator_t* const i_allocator)
{
    linear_allocator_t allocator = create_linear_allocator(i_allocator, "Main Dialog Allocator", SIZE_MB(6));

    HWND hDialogWnd = pxCreateDialogParam(i_hInstance, MAKEINTRESOURCE(IDD_DLG_MAIN), NULL, (DLGPROC)MainDialogProc, NULL);
    if (!hDialogWnd)
    {
        return NULL;
    }

    const BOOL useDarkMode = TRUE;
    pxDwmSetWindowAttribute(hDialogWnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(BOOL));

    s_mainDlgState.hInstance = i_hInstance;
    s_mainDlgState.hWnd = hDialogWnd;
    s_mainDlgState.lblAllocsCount = pxGetDlgItem(hDialogWnd, IDC_ALLOCS_COUNT);
    s_mainDlgState.lblFreesCount = pxGetDlgItem(hDialogWnd, IDC_FREES_COUNT);
    s_mainDlgState.lblUsedMemory = pxGetDlgItem(hDialogWnd, IDC_USED_MEMORY);
    s_mainDlgState.hWnds = create_inplace_array<HWND, 2>();
    s_mainDlgState.allocator = allocator;

    return hDialogWnd;
}

void UIMainDialogRun()
{
    TryInitializingTrayIconAndTaskBarWidget();
    pxShowWindow(s_mainDlgState.hWnd, SW_HIDE);

    MSG msg = {};
    while (pxGetMessage(&msg, NULL, 0, 0))
    {
        bool processed = false;
        for (ssize i = 0; i < s_mainDlgState.hWnds.size; i++)
        {
            if (pxIsDialogMessage(s_mainDlgState.hWnds[i], &msg))
            {
                processed = true;
                break;
            }
        }

        if (!processed)
        {
            pxTranslateMessage(&msg);
            pxDispatchMessage(&msg);
        }
    }
}

void UIMainDialogCleanUp()
{
    UIWidgetCleanUp();
    UITrayIconCleanUp();
    pxDestroyWindow(s_mainDlgState.hWnd);
}
