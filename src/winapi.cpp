#include "winapi.h"

#include <floral/assert.h>
#include <floral/log.h>
#include <floral/thread_context.h>

static DWORD GetWin32ErrorCodeFromHResult(const HRESULT i_hResult)
{
    FLORAL_ASSERT(i_hResult != S_OK);
    if (HRESULT(i_hResult & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0))
    {
        return HRESULT_CODE(i_hResult);
    }

    // Not a Win32 HRESULT so return a generic error code.
    return ERROR_CAN_NOT_COMPLETE;
}

static void AssertHResultOK(const HRESULT i_hResult)
{
    if (i_hResult != S_OK)
    {
        scratch_region_t scratch = thread_scratch_begin();
        tchar* errorMsg = arena_push_podarr(scratch.arena, tchar, FLORAL_MAX_LOCAL_BUFFER_LENGTH);
        DWORD errorCode = GetWin32ErrorCodeFromHResult(i_hResult);
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                      errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      errorMsg, FLORAL_MAX_LOCAL_BUFFER_LENGTH,
                      NULL);
        LOG_ERROR(TEXT("API error:\n"
                       "  - Win32 ErrorCode: 0x%x\n"
                       "  - HRESULT ErrorCode: 0x%x\n"
                       "Error message: %s"),
                  errorCode, i_hResult, errorMsg);
        thread_scratch_end(&scratch);
        FLORAL_DEBUG_BREAK();
    }
}

template <typename TTarget, typename TValue>
static void AssertNotEqual(const TTarget i_target, const TValue i_value)
{
    if (i_target == (const TTarget)i_value)
    {
        scratch_region_t scratch = thread_scratch_begin();
        tchar* errorMsg = arena_push_podarr(scratch.arena, tchar, FLORAL_MAX_LOCAL_BUFFER_LENGTH);
        DWORD errorCode = GetLastError();
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                      errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      errorMsg, FLORAL_MAX_LOCAL_BUFFER_LENGTH,
                      NULL);
        LOG_ERROR(TEXT("API error:\n"
                       "  - Win32 ErrorCode: 0x%x\n"
                       "Error message: %s"),
                  errorCode, errorMsg);
        thread_scratch_end(&scratch);
        FLORAL_DEBUG_BREAK();
    }
}

static void AssertBoolResult(const BOOL i_result)
{
    AssertNotEqual(i_result, FALSE);
}

// ----------------------------------------------------------------------------

void pxCoCreateGuid(GUID* pguid)
{
    HRESULT result = CoCreateGuid(pguid);
    AssertHResultOK(result);
}

void pxCoInitializeEx(LPVOID pvReserved, DWORD dwCoInit)
{
    HRESULT result = CoInitializeEx(pvReserved, dwCoInit);
    AssertHResultOK(result);
}

void pxCoInitializeSecurity(PSECURITY_DESCRIPTOR pSecDesc, LONG cAuthSvc, SOLE_AUTHENTICATION_SERVICE* asAuthSvc, void* pReserved1, DWORD dwAuthnLevel, DWORD dwImpLevel, void* pAuthList, DWORD dwCapabilities, void* pReserved3)
{
    HRESULT result = CoInitializeSecurity(pSecDesc, cAuthSvc, asAuthSvc, pReserved1, dwAuthnLevel, dwImpLevel, pAuthList, dwCapabilities, pReserved3);
    AssertHResultOK(result);
}

void pxCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
{
    HRESULT result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    AssertHResultOK(result);
}

void pxCoUninitialize()
{
    CoUninitialize();
}

void pxLoadIconMetric(HINSTANCE hinst, PCWSTR pszName, int lims, HICON* phico)
{
    HRESULT result = LoadIconMetric(hinst, pszName, lims, phico);
    AssertHResultOK(result);
}

void pxSetProcessDpiAwareness(PROCESS_DPI_AWARENESS value)
{
    HRESULT result = SetProcessDpiAwareness(value);
    AssertHResultOK(result);
}

void pxDwmSetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute)
{
    HRESULT result = DwmSetWindowAttribute(hwnd, dwAttribute, pvAttribute, cbAttribute);
    AssertHResultOK(result);
}

// ----------------------------------------------------------------------------

void pxDestroyIcon(HICON hIcon)
{
    BOOL result = DestroyIcon(hIcon);
    AssertBoolResult(result);
}

void pxGetCursorPos(LPPOINT lpPoint)
{
    BOOL result = GetCursorPos(lpPoint);
    AssertBoolResult(result);
}

void pxInsertMenu(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCWSTR lpNewItem)
{
    BOOL result = InsertMenu(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);
    AssertBoolResult(result);
}

void pxSetForegroundWindow(HWND hWnd)
{
    BOOL result = SetForegroundWindow(hWnd);
    AssertBoolResult(result);
}

void pxTrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, const RECT* prcRect)
{
    BOOL result = TrackPopupMenu(hMenu, uFlags, x, y, nReserved, hWnd, prcRect);
    AssertBoolResult(result);
}

void pxDestroyMenu(HMENU hMenu)
{
    BOOL result = DestroyMenu(hMenu);
    AssertBoolResult(result);
}

void pxShowWindow(HWND hWnd, int nCmdShow)
{
    ShowWindow(hWnd, nCmdShow);
}

void pxSetPriorityClass(HANDLE hProcess, DWORD dwPriorityClass)
{
    BOOL result = SetPriorityClass(hProcess, dwPriorityClass);
    AssertBoolResult(result);
}

void pxDestroyWindow(HWND hWnd)
{
    BOOL result = DestroyWindow(hWnd);
    AssertBoolResult(result);
}

void pxCloseHandle(HANDLE hObject)
{
    BOOL result = CloseHandle(hObject);
    AssertBoolResult(result);
}

void pxDeleteObject(HGDIOBJ ho)
{
    BOOL result = DeleteObject(ho);
    FLORAL_ASSERT_MSG(result != 0, "The specified handle is not valid or is currently selected into a DC");
}

void pxGetWindowRect(HWND hWnd, LPRECT lpRect)
{
    BOOL result = GetWindowRect(hWnd, lpRect);
    AssertBoolResult(result);
}

void pxMoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
    BOOL result = MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);
    AssertBoolResult(result);
}

void pxUpdateLayeredWindow(HWND hWnd, HDC hdcDst, POINT* pptDst, SIZE* psize, HDC hdcSrc, POINT* pptSrc, COLORREF crKey, BLENDFUNCTION* pblend, DWORD dwFlags)
{
    BOOL result = UpdateLayeredWindow(hWnd, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend, dwFlags);
    AssertBoolResult(result);
}

void pxInvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase)
{
    BOOL result = InvalidateRect(hWnd, lpRect, bErase);
    FLORAL_ASSERT(result);
}

void pxKillTimer(HWND hWnd, UINT_PTR uIDEvent)
{
    BOOL result = KillTimer(hWnd, uIDEvent);
    AssertBoolResult(result);
}

void pxDeleteDC(HDC hdc)
{
    BOOL result = DeleteDC(hdc);
    FLORAL_ASSERT(result != FALSE);
}

// ----------------------------------------------------------------------------

BOOL pxShell_NotifyIcon(DWORD dwMessage, PNOTIFYICONDATA lpData)
{
    BOOL result = Shell_NotifyIcon(dwMessage, lpData);
    if (dwMessage == NIM_SETVERSION)
    {
        if (result == FALSE)
        {
            LOG_ERROR("API error: The request version is not supported");
            FLORAL_DEBUG_BREAK();
        }
    }
    return result;
}

HMENU pxCreatePopupMenu()
{
    HMENU handle = CreatePopupMenu();
    AssertNotEqual(handle, nullptr);
    return handle;
}

LONG_PTR pxSetWindowLongPtr(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
{
    // TODO: error checking logic: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowlongptrw
    return SetWindowLongPtr(hWnd, nIndex, dwNewLong);
}

HANDLE pxGetCurrentProcess()
{
    return GetCurrentProcess();
}

HWND pxCreateDialogParam(HINSTANCE hInstance, LPTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
    HWND handle = CreateDialogParamW(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
    AssertNotEqual(handle, nullptr);
    return handle;
}

HWND pxGetDlgItem(HWND hDlg, int nIDDlgItem)
{
    HWND handle = GetDlgItem(hDlg, nIDDlgItem);
    AssertNotEqual(handle, nullptr);
    return handle;
}

BOOL pxGetMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
    BOOL value = GetMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    AssertNotEqual(value, -1);
    return value;
}

BOOL pxIsDialogMessage(HWND hDlg, LPMSG lpMsg)
{
    return IsDialogMessage(hDlg, lpMsg);
}

BOOL pxTranslateMessage(const MSG* lpMsg)
{
    return TranslateMessage(lpMsg);
}

LRESULT pxDispatchMessage(const MSG* lpMsg)
{
    return DispatchMessage(lpMsg);
}

HMODULE pxLoadLibrary(LPCTSTR lpLibFileName)
{
    HMODULE handle = LoadLibrary(lpLibFileName);
    AssertNotEqual(handle, nullptr);
    return handle;
}

FARPROC pxGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    FARPROC ptr = GetProcAddress(hModule, lpProcName);
    AssertNotEqual(ptr, nullptr);
    return ptr;
}

HANDLE pxCreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    HANDLE handle = CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    AssertNotEqual(handle, INVALID_HANDLE_VALUE);
    return handle;
}

DWORD pxGetCurrentThreadId()
{
    return GetCurrentThreadId();
}

DWORD pxGetCurrentProcessId()
{
    return GetCurrentProcessId();
}

LPTOP_LEVEL_EXCEPTION_FILTER pxSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
    return SetUnhandledExceptionFilter(lpTopLevelExceptionFilter);
}

HBITMAP pxCreateDIBSection(HDC hdc, const BITMAPINFO* pbmi, UINT usage, VOID** ppvBits, HANDLE hSection, DWORD offset)
{
    HBITMAP handle = CreateDIBSection(hdc, pbmi, usage, ppvBits, hSection, offset);
    AssertNotEqual(handle, nullptr);
    return handle;
}

HGDIOBJ pxSelectObject(HDC hdc, HGDIOBJ h)
{
    HGDIOBJ handle = SelectObject(hdc, h);
    FLORAL_ASSERT(handle != nullptr && handle != HGDI_ERROR);
    return handle;
}

HDC pxGetDC(HWND hWnd)
{
    HDC handle = GetDC(hWnd);
    FLORAL_ASSERT(handle != nullptr);
    return handle;
}

int pxReleaseDC(HWND hWnd, HDC hDC)
{
    return ReleaseDC(hWnd, hDC);
}

HGDIOBJ pxGetStockObject(int i)
{
    HGDIOBJ handle = GetStockObject(i);
    FLORAL_ASSERT(handle != nullptr);
    return handle;
}

HWND pxFindWindow(LPCTSTR lpClassName, LPCTSTR lpWindowName)
{
    HWND handle = FindWindow(lpClassName, lpWindowName);
    AssertNotEqual(handle, nullptr);
    return handle;
}

HWND pxFindWindowEx(HWND hWndParent, HWND hWndChildAfter, LPCTSTR lpszClass, LPCTSTR lpszWindow)
{
    HWND handle = FindWindowEx(hWndParent, hWndChildAfter, lpszClass, lpszWindow);
    AssertNotEqual(handle, nullptr);
    return handle;
}

HWND pxSetParent(HWND hWndChild, HWND hWndNewParent)
{
    HWND handle = SetParent(hWndChild, hWndNewParent);
    AssertNotEqual(handle, nullptr);
    return handle;
}

HDC pxCreateCompatibleDC(HDC hdc)
{
    HDC handle = CreateCompatibleDC(hdc);
    FLORAL_ASSERT(handle != nullptr);
    return handle;
}

UINT_PTR pxSetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc)
{
    UINT_PTR value = SetTimer(hWnd, nIDEvent, uElapse, lpTimerFunc);
    AssertNotEqual(value, 0);
    return value;
}

// ----------------------------------------------------------------------------

void pxPostQuitMessage(int nExitCode)
{
    PostQuitMessage(nExitCode);
}
