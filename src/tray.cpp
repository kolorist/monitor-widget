#include "tray.h"

#include <resource.h>

#include <floral/assert.h>
#include <floral/log.h>
#include <floral/misc.h>

#include "winapi.h"
#include "configs.h"
#include "defines.h"

struct TrayIconState s_trayIconState;

// ----------------------------------------------------------------------------

class __declspec(uuid("5E506BA9-B79C-4CF7-ABDC-5999F8478F1A")) AppTrayIcon;

bool UITrayIconInitialize(const HINSTANCE i_appInstance, HWND i_parentWnd)
{
    LOG_SCOPE(trayicon);
    if (s_trayIconState.ready)
    {
        return true;
    }

    NOTIFYICONDATA niData = {
        .cbSize = sizeof(niData),
        .hWnd = i_parentWnd,
        .uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP,
        .uCallbackMessage = AM_TRAY
    };

    pxLoadIconMetric(i_appInstance, MAKEINTRESOURCE(IDI_APPICON), LIM_LARGE, &niData.hIcon);
    tcstr_xcopy(niData.szTip, 64, LITERAL("monitor-widget"));

    BOOL result = pxShell_NotifyIcon(NIM_ADD, &niData);
    if (result)
    {
        LOG_DEBUG("TrayIcon initialized.");
        s_trayIconState.ready = true;
        return true;
    }
    else
    {
        LOG_WARNING("Tray Icon failed to initialize, mostly likely due to Explorer.exe haven't started yet. Will retry later.");
        return false;
    }
}

// ----------------------------------------------------------------------------

void UITrayIconCleanUp()
{
    if (s_trayIconState.ready)
    {
        NOTIFYICONDATA niData = { .cbSize = sizeof(niData) };
        niData.uFlags = NIF_GUID;
        niData.guidItem = __uuidof(AppTrayIcon);
        pxShell_NotifyIcon(NIM_DELETE, &niData);

        s_trayIconState.ready = false;
        LOG_DEBUG("TrayIcon destroyed.");
    }
}

// ----------------------------------------------------------------------------

void UITrayIconHandleMessage(const u32 i_message, const HWND i_hwnd)
{
    if (!s_trayIconState.ready)
    {
        return;
    }

    switch (i_message)
    {
    case WM_LBUTTONDBLCLK:
    {
        SendMessage(i_hwnd, WM_COMMAND, ID_SWITCH_MODE, 0);
        break;
    }

    case WM_RBUTTONUP:
    case WM_CONTEXTMENU:
    {
        POINT pt = {};
        pxGetCursorPos(&pt);

        HMENU hContextMenu = pxCreatePopupMenu();
        if (hContextMenu)
        {
            pxInsertMenu(hContextMenu, IDM_TRAY_SHOW_IN_TASKBAR,
                         CFGGetBool(CFGKey::ShowInTaskBar) ? (MF_BYCOMMAND | MF_CHECKED)
                                                           : (MF_BYCOMMAND | MF_UNCHECKED),
                         ID_TOGGLE_TASKBAR_INFO, LITERAL("Show In Taskbar"));
            pxInsertMenu(hContextMenu, IDM_TRAY_START_ON_BOOT,
                         CFGGetBool(CFGKey::StartOnBoot) ? (MF_BYCOMMAND | MF_CHECKED)
                                                         : (MF_BYCOMMAND | MF_UNCHECKED),
                         ID_START_ON_BOOT, LITERAL("Start On Boot"));
            pxInsertMenu(hContextMenu, IDM_TRAY_RESTORE, MF_BYCOMMAND, ID_RESTORE_FROM_TRAY, LITERAL("Restore"));
            pxInsertMenu(hContextMenu, IDM_TRAY_SWITCH_MODE, MF_BYCOMMAND, ID_SWITCH_MODE, LITERAL("Switch Mode"));
            pxInsertMenu(hContextMenu, IDM_TRAY_EXIT, MF_BYCOMMAND, ID_EXIT, LITERAL("Exit"));

#if defined(DEBUG_BUILD)
            pxInsertMenu(hContextMenu, -1, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
            pxInsertMenu(hContextMenu, -1, MF_BYPOSITION, ID_FORCE_CRASH, LITERAL("Force Crash!"));
#endif

            pxSetForegroundWindow(i_hwnd);
            pxTrackPopupMenu(hContextMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, i_hwnd, NULL);
            pxDestroyMenu(hContextMenu);
        }

        break;
    }

    default:
        break;
    }
}
