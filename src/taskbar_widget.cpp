#include "taskbar_widget.h"

#include <floral/stdaliases.h>
#include <floral/assert.h>
#include <floral/log.h>
#include <floral/misc.h>
#include <floral/time.h>
#include <floral/thread_context.h>

#include <resource.h>

#include "renderer_gdiplus.h"
#include "winapi.h"
#include "defines.h"
#include "monitor/cpu.h"
#include "monitor/gpu.h"
#include "monitor/network.h"
#include "scripting.h"
#include "files_tracker.h"
#include "utils.h"

static OSTaskBarState s_taskBarState;
static RNDState s_rndState;
static UIWidgetSurfaceState s_surfaceState;
static UIWidgetState s_widgetState;

// ----------------------------------------------------------------------------

constexpr s32 k_refDpi = 96;
constexpr u32 k_defaultUpdateInterval = 1000;

// ----------------------------------------------------------------------------

struct ScriptingOnInitializeCallContext : SCRClosure
{
    s32 PushArgs(lua_State*)
    {
        return 0;
    }
    void DeserializeReturnValues(lua_State*) {}
};

struct ScriptingOnUpdateCallContext : SCRClosure
{
    s32 w;
    s32 h;

    s32 PushArgs(lua_State* i_vm)
    {
        lua_pushnumber(i_vm, w);
        lua_pushnumber(i_vm, h);
        return 2;
    }
    void DeserializeReturnValues(lua_State*) {}
};

struct ScriptingOnWidgetResizedCallContext : SCRClosure
{
    // arguments
    s32 w;
    s32 h;
    // setup
    s32 PushArgs(lua_State* i_vm)
    {
        lua_pushnumber(i_vm, w);
        lua_pushnumber(i_vm, h);
        return 2;
    }
    void DeserializeReturnValues(lua_State* i_vm)
    {
        MARK_UNUSED(i_vm);
    }
};

struct ScriptingSwitchModeCallContext : SCRClosure
{
    s32 PushArgs(lua_State*)
    {
        return 0;
    }
    void DeserializeReturnValues(lua_State*) {}
};

static s32 ScriptingSetUpdateInterval(lua_State* i_vm)
{
    LOG_SCOPE(lua2cpp);
    const UIWidgetState* const state = (UIWidgetState*)lua_touserdata(i_vm, lua_upvalueindex(1));

    const s32 nArgs = lua_gettop(i_vm);
    FLORAL_ASSERT(nArgs == 1);
    const s32 interval = (s32)lua_tointeger(i_vm, 1);
    pxSetTimer(state->hwnd, ID_TASKBAR_TIMER, interval, NULL);
    LOG_INFO("Update interval set to %d ms", interval);

    return 0;
}

static s32 ScriptingSetWidgetSize(lua_State* i_vm)
{
    LOG_SCOPE(lua2cpp);
    UIWidgetState* const state = (UIWidgetState*)lua_touserdata(i_vm, lua_upvalueindex(1));
    const s32 nArgs = lua_gettop(i_vm);
    FLORAL_ASSERT(nArgs == 2);
    state->landscapeLength = (s32)lua_tointeger(i_vm, 1);
    state->portraitLength = (s32)lua_tointeger(i_vm, 2);
    LOG_INFO("Widget size set to %d landscape, %d portrait", state->landscapeLength, state->portraitLength);
    return 0;
}

static s32 ScriptingGetProcessorUtilization(lua_State* i_vm)
{
    scratch_region_t scratch = thread_scratch_begin();
    f32 avgLoad = 0.0f;
    cpu::ReadProcessorUtilization(&avgLoad, nullptr, nullptr, 0);
    lua_pushnumber(i_vm, avgLoad);
    thread_scratch_end(&scratch);
    return 1;
}

static s32 ScriptingGetProcessorTemperature(lua_State* i_vm)
{
    scratch_region_t scratch = thread_scratch_begin();
    f32 packageTemp = 0.0f;
    cpu::ReadProcessorTemperature(&packageTemp, nullptr, nullptr, 0);
    lua_pushnumber(i_vm, packageTemp);
    thread_scratch_end(&scratch);
    return 1;
}

static s32 ScriptingGetRAMUtilization(lua_State* i_vm)
{
    lua_createtable(i_vm, 0, 2);

    s32 physicalLoad = 0;
    s32 virtualLoad = 0;
    cpu::ReadMemoryUtilization(&physicalLoad, &virtualLoad);

    lua_pushstring(i_vm, "physicalLoad");
    lua_pushnumber(i_vm, physicalLoad);
    lua_settable(i_vm, -3);

    lua_pushstring(i_vm, "virtualLoad");
    lua_pushnumber(i_vm, virtualLoad);
    lua_settable(i_vm, -3);

    return 1;
}

static s32 ScriptingGetGPUUtilization(lua_State* i_vm)
{
    u32 geLoad = 0;
    u32 fbLoad = 0;
    u32 vidLoad = 0;
    u32 busLoad = 0;
    gpu::ReadUtilization(&geLoad, &fbLoad, &vidLoad, &busLoad);

    lua_createtable(i_vm, 0, 4);
    lua_pushstring(i_vm, "graphicsEngineLoad");
    lua_pushnumber(i_vm, geLoad);
    lua_settable(i_vm, -3);

    lua_pushstring(i_vm, "framebufferLoad");
    lua_pushinteger(i_vm, fbLoad);
    lua_settable(i_vm, -3);

    lua_pushstring(i_vm, "videoLoad");
    lua_pushinteger(i_vm, vidLoad);
    lua_settable(i_vm, -3);

    lua_pushstring(i_vm, "busLoad");
    lua_pushinteger(i_vm, busLoad);
    lua_settable(i_vm, -3);

    return 1;
}

static s32 ScriptingGetGPUTemperature(lua_State* i_vm)
{
    f32 temp = 0.0f;
    gpu::ReadTemperature(&temp);
    lua_pushnumber(i_vm, temp);
    return 1;
}

static s32 ScriptingGetVRAMUtilization(lua_State* i_vm)
{
    u32 usage = 0;
    gpu::ReadVRAMUtilization(&usage);
    lua_pushinteger(i_vm, usage);
    return 1;
}

static s32 ScriptingGetNetworkStats(lua_State* i_vm)
{
    f32 ingress = 0.0f;
    f32 egress = 0.0f;
    u64 sentBytes = 0;
    u64 receivedBytes = 0;
    network::ReadStats(&ingress, &egress, &sentBytes, &receivedBytes);

    lua_createtable(i_vm, 0, 4);
    lua_pushstring(i_vm, "sent");
    lua_pushnumber(i_vm, (f64)sentBytes);
    lua_settable(i_vm, -3);

    lua_pushstring(i_vm, "received");
    lua_pushnumber(i_vm, (f64)receivedBytes);
    lua_settable(i_vm, -3);

    lua_pushstring(i_vm, "egress");
    lua_pushnumber(i_vm, egress);
    lua_settable(i_vm, -3);

    lua_pushstring(i_vm, "ingress");
    lua_pushnumber(i_vm, ingress);
    lua_settable(i_vm, -3);

    return 1;
}

// ----------------------------------------------------------------------------

static void UpdateSurface(HWND i_hwnd)
{
    // Check the DPI and where we are right now
    RECT taskbarRect;
    RECT trayRect;
    pxGetWindowRect(s_taskBarState.hTaskBarWnd, &taskbarRect);
    pxGetWindowRect(s_taskBarState.hTrayNotifyWnd, &trayRect);
    const s32 dpi = OSGetDPI(i_hwnd);
    const vec2i tbSize = vec2i(taskbarRect.right - taskbarRect.left, taskbarRect.bottom - taskbarRect.top);

    // We won't do anything if there is no space to render into
    if (tbSize.x <= 0 || tbSize.y <= 0)
    {
        return;
    }

    // We also won't do anything if we don't know where the tray area is
    bool isTrayRectValid = (trayRect.right >= trayRect.left);
    isTrayRectValid &= (trayRect.bottom >= trayRect.top);
    isTrayRectValid &= (trayRect.left >= taskbarRect.left && trayRect.right <= taskbarRect.right);
    isTrayRectValid &= (trayRect.top >= taskbarRect.top && trayRect.bottom <= taskbarRect.bottom);
    if (!isTrayRectValid)
    {
        LOG_WARNING("TaskBar contains invalid bounding rects:");
        LOG_WARNING("  - Shell_TrayWnd: (%d; %d; %d; %d)", taskbarRect.left, taskbarRect.top,
                    taskbarRect.right, taskbarRect.bottom);
        LOG_WARNING("  - TrayNotifyWnd: (%d; %d; %d; %d)", trayRect.left, trayRect.top,
                    trayRect.right, trayRect.bottom);
        return;
    }

    // recalculate the origin, orientation and size of the widget
    Orientation orient = Orientation::Landscape;
    vec2i widgetOrigin = vec2i::zero;
    vec2i widgetSize = vec2i::zero;
    if (tbSize.x > tbSize.y)
    {
        widgetSize.x = MulDiv(s_widgetState.landscapeLength, dpi, k_refDpi);
        widgetSize.y = tbSize.y;
        widgetOrigin = vec2i(trayRect.left - widgetSize.x, 0);
        orient = Orientation::Landscape;
    }
    else
    {
        widgetSize.x = tbSize.x;
        widgetSize.y = MulDiv(s_widgetState.portraitLength, dpi, k_refDpi);
        widgetOrigin = vec2i(0, trayRect.top - widgetSize.y);
        orient = Orientation::Portrait;
    }

    // widget's draw area cannot be zero
    if (widgetSize.x <= 0 || widgetSize.y <= 0)
    {
        return;
    }

    // 1st phase: origin changes, move the window
    if (s_widgetState.origin != widgetOrigin)
    {
        pxMoveWindow(i_hwnd, widgetOrigin.x, widgetOrigin.y, widgetSize.x, widgetSize.y, TRUE);
        s_widgetState.origin = widgetOrigin;
    }

    // 2nd phase: orientation changes
    if (s_widgetState.orientation != orient)
    {
        s_widgetState.orientation = orient;
    }

    // 3rd phase: dpi and size change, recreate the render surface and the renderer
    if (s_surfaceState.dpi != dpi || s_surfaceState.size != widgetSize)
    {
        // recreate surface
        BITMAPINFO bmInfo = {
            .bmiHeader = {
                          .biSize = sizeof(bmInfo.bmiHeader),
                          .biWidth = widgetSize.x,
                          .biHeight = widgetSize.y,
                          .biPlanes = 1,
                          .biBitCount = 32,
                          .biCompression = BI_RGB,
                          .biSizeImage = 0,
                          .biXPelsPerMeter = 0,
                          .biYPelsPerMeter = 0,
                          .biClrUsed = BI_RGB,
                          .biClrImportant = 0 }
        };

        if (s_surfaceState.buffer != NULL)
        {
            LOG_DEBUG("Deleting old bitmap surface: 0x%x", (aptr)s_surfaceState.buffer);
            pxDeleteObject(s_surfaceState.buffer);
        }
        s_surfaceState.buffer = pxCreateDIBSection(s_surfaceState.hdc, &bmInfo, DIB_RGB_COLORS, &s_surfaceState.bufferData, NULL, 0);
        pxSelectObject(s_surfaceState.hdc, s_surfaceState.buffer);
        LOG_DEBUG("Created new bitmap surface: 0x%x (%d x %d)", (aptr)s_surfaceState.buffer, widgetSize.x, widgetSize.y);

        // refresh the renderer
        const f32 dpiScale = (f32)dpi / k_refDpi;
        RNDRefresh(&s_rndState, s_surfaceState.hdc, widgetSize, dpiScale);

        s_surfaceState.dpi = dpi;
        s_surfaceState.size = widgetSize;
    }
}

static void Present(HWND i_widgetHwnd, HWND i_taskbarHwnd)
{
    HDC taskbarHDC = pxGetDC(i_taskbarHwnd);

    POINT srcOrigin = { s_widgetState.origin.x, s_widgetState.origin.y };
    SIZE srcSize = { s_surfaceState.size.x, s_surfaceState.size.y };
    POINT zero = {};
    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;
    blend.BlendFlags = 0;

    pxUpdateLayeredWindow(i_widgetHwnd, taskbarHDC, &srcOrigin, &srcSize, s_surfaceState.hdc, &zero, RGB(0, 0, 0), &blend, ULW_ALPHA);

    pxReleaseDC(i_taskbarHwnd, taskbarHDC);
}

// ----------------------------------------------------------------------------

static INT_PTR TaskbarDlgProc(HWND i_hwnd, UINT i_msg, WPARAM i_wParam, LPARAM i_lParam)
{
    s32 msgResult = 0;
    INT_PTR dlgResult = FALSE;

    switch (i_msg)
    {
    // case WM_DPICHANGED:
    // case WM_DPICHANGED_BEFOREPARENT:
    case WM_DPICHANGED_AFTERPARENT:
    case WM_SIZE:
    {
        // trigger WM_PAINT immediately, this will update the app with the new DPI
        pxInvalidateRect(i_hwnd, NULL, TRUE);
        dlgResult = TRUE;
        break;
    }

    case WM_INITDIALOG:
    {
        pxSetWindowLongPtr(i_hwnd, GWLP_USERDATA, (LONG_PTR)i_lParam);
        dlgResult = TRUE;
        break;
    }

    case WM_PAINT:
    {
        // Update surface and renderer if necessary
        UpdateSurface(i_hwnd);

        // Reload the scripting system if we detect any script changes
        if (FTHasChanges())
        {
            FTLock();
            if (SCRReloadVMThread())
            {
                SCRRegisterFunc(&ScriptingSetUpdateInterval, "set_update_interval", &s_widgetState);
                SCRRegisterFunc(&ScriptingSetWidgetSize, "set_widget_size", &s_widgetState);

                SCRRegisterFunc(&ScriptingGetNetworkStats, "get_network_stats", nullptr);

                SCRRegisterFunc(&ScriptingGetProcessorUtilization, "get_processor_utilization", nullptr);
                SCRRegisterFunc(&ScriptingGetProcessorTemperature, "get_processor_temperature", nullptr);
                SCRRegisterFunc(&ScriptingGetRAMUtilization, "get_ram_utilization", nullptr);

                SCRRegisterFunc(&ScriptingGetGPUUtilization, "get_gpu_utilization", nullptr);
                SCRRegisterFunc(&ScriptingGetGPUTemperature, "get_gpu_temperature", nullptr);
                SCRRegisterFunc(&ScriptingGetVRAMUtilization, "get_vram_utilization", nullptr);

                RNDDestroyAllResources(&s_rndState);
                RNDBindScriptingAPIs(&s_rndState);
                ScriptingOnInitializeCallContext callCtx = {};
                SCRCallFunc("on_initialize", &callCtx);
            }
            FTUnlock();
        }

        cpu::UpdateOSPerfCounters();

        if (RNDBeginRender(&s_rndState))
        {
            ScriptingOnUpdateCallContext callCtx = {
                .w = MulDiv(s_surfaceState.size.x, k_refDpi, s_surfaceState.dpi),
                .h = MulDiv(s_surfaceState.size.y, k_refDpi, s_surfaceState.dpi)
            };
            SCRCallFunc("on_update", &callCtx);

            RNDEndRender(&s_rndState);
            Present(i_hwnd, s_taskBarState.hTaskBarWnd);
        }

        // since we are not actually use the WM_PAINT message, we will let the default DlgProc
        // handle it by returning FALSE
        break;
    }

    case WM_CTLCOLORDLG:
    {
        dlgResult = (INT_PTR)pxGetStockObject(BLACK_BRUSH);
        break;
    }

    case WM_TIMER:
    {
        switch (LOWORD(i_wParam))
        {
        case ID_TASKBAR_TIMER:
        {
            pxInvalidateRect(i_hwnd, NULL, TRUE);
            dlgResult = TRUE;
            break;
        }

        default:
            break;
        }

        break;
    }

    default:
        break;
    }

    SetWindowLong(i_hwnd, DWLP_MSGRESULT, msgResult);
    return dlgResult;
}

// ----------------------------------------------------------------------------

HWND UIWidgetInitialize(const HINSTANCE i_appInstance, linear_allocator_t* const i_allocator)
{
    LOG_SCOPE(widget);
    if (s_widgetState.ready)
    {
        return NULL;
    }

    // locate the taskbar
    s_taskBarState.hTaskBarWnd = pxFindWindow(TEXT("Shell_TrayWnd"), NULL);
    s_taskBarState.hReBarWnd = pxFindWindowEx(s_taskBarState.hTaskBarWnd, NULL, TEXT("ReBarWindow32"), NULL);
    s_taskBarState.hTaskWnd = pxFindWindowEx(s_taskBarState.hReBarWnd, NULL, TEXT("MSTaskSwWClass"), NULL);
    s_taskBarState.hTrayNotifyWnd = pxFindWindowEx(s_taskBarState.hTaskBarWnd, NULL, TEXT("TrayNotifyWnd"), NULL);

    // dialog for presentation
    s_widgetState.landscapeLength = 100;
    s_widgetState.portraitLength = 100;
    s_widgetState.hwnd = pxCreateDialogParam(i_appInstance, MAKEINTRESOURCE(IDD_DLG_TASKBAR_SMALL), NULL,
                                             (DLGPROC)TaskbarDlgProc, (LPARAM)&s_widgetState);
    pxSetParent(s_widgetState.hwnd, s_taskBarState.hTaskBarWnd);

    const s32 dpi = OSGetDPI(s_widgetState.hwnd);
    LOG_DEBUG("Initial DPI: %d", dpi);

    s_surfaceState.hdc = pxCreateCompatibleDC(pxGetDC(s_widgetState.hwnd));
    s_surfaceState.buffer = NULL;
    s_surfaceState.bufferData = nullptr;
    RNDInitialize(&s_rndState, i_appInstance, s_surfaceState.hdc, i_allocator);
    pxSetTimer(s_widgetState.hwnd, ID_TASKBAR_TIMER, k_defaultUpdateInterval, NULL);

    s_widgetState.ready = true;
    LOG_DEBUG("Widget initialized");

    return s_widgetState.hwnd;
}

// ----------------------------------------------------------------------------

void UIWidgetCleanUp()
{
    if (!s_widgetState.ready)
    {
        return;
    }

    pxKillTimer(s_widgetState.hwnd, ID_TASKBAR_TIMER);

    RNDCleanUp(&s_rndState);

    pxDeleteDC(s_surfaceState.hdc);
    pxDeleteObject(s_surfaceState.buffer);

    pxDestroyWindow(s_widgetState.hwnd);

    s_widgetState.ready = false;
    LOG_DEBUG("TaskBarWidget destroyed.");
}

// ----------------------------------------------------------------------------

bool UIWidgetToggle(const bool i_visible)
{
    if (!s_widgetState.ready)
    {
        LOG_WARNING("Taskbar Widget didn't initialize properly");
        return false;
    }

    pxInvalidateRect(s_widgetState.hwnd, NULL, TRUE);
    pxShowWindow(s_widgetState.hwnd, i_visible ? SW_SHOWNOACTIVATE : SW_HIDE);
    return true;
}

void UIWidgetSwitchMode()
{
    ScriptingSwitchModeCallContext callCtx = {};
    SCRCallFunc("on_mode_switched", &callCtx);
    pxInvalidateRect(s_widgetState.hwnd, NULL, TRUE);
}
