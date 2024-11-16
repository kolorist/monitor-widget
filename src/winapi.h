#pragma once

#include <Windows.h>
#include <CommCtrl.h>
#include <ShellScalingApi.h>
#include <combaseapi.h>
#include <shellapi.h>
#include <errhandlingapi.h>
#include <minidumpapiset.h>
#include <taskschd.h>
#include <dwmapi.h>

// HRESULT
void pxCoCreateGuid(GUID* pguid);
void pxCoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);
void pxCoInitializeSecurity(PSECURITY_DESCRIPTOR pSecDesc, LONG cAuthSvc, SOLE_AUTHENTICATION_SERVICE* asAuthSvc, void* pReserved1, DWORD dwAuthnLevel, DWORD dwImpLevel, void* pAuthList, DWORD dwCapabilities, void* pReserved3);
void pxCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv);
void pxCoUninitialize();
void pxDestroyIcon(HICON hIcon);
void pxLoadIconMetric(HINSTANCE hinst, PCWSTR pszName, int lims, HICON* phico);
void pxSetProcessDpiAwareness(PROCESS_DPI_AWARENESS value);
void pxDwmSetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);

// BOOL
void pxDestroyIcon(HICON hIcon);
void pxGetCursorPos(LPPOINT lpPoint);
void pxInsertMenu(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCWSTR lpNewItem);
void pxSetForegroundWindow(HWND hWnd);
void pxTrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, const RECT* prcRect);
void pxDestroyMenu(HMENU hMenu);
void pxShowWindow(HWND hWnd, int nCmdShow);
void pxSetPriorityClass(HANDLE hProcess, DWORD dwPriorityClass);
void pxDestroyWindow(HWND hWnd);
void pxCloseHandle(HANDLE hObject);
void pxDeleteObject(HGDIOBJ ho);
void pxGetWindowRect(HWND hWnd, LPRECT lpRect);
void pxMoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
void pxUpdateLayeredWindow(HWND hWnd, HDC hdcDst, POINT* pptDst, SIZE* psize, HDC hdcSrc, POINT* pptSrc, COLORREF crKey, BLENDFUNCTION* pblend, DWORD dwFlags);
void pxInvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase);
void pxKillTimer(HWND hWnd, UINT_PTR uIDEvent);
void pxDeleteDC(HDC hdc);

// has return value
BOOL pxShell_NotifyIcon(DWORD dwMessage, PNOTIFYICONDATA lpData);
HMENU pxCreatePopupMenu();
LONG_PTR pxSetWindowLongPtr(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
HANDLE pxGetCurrentProcess();
HWND pxCreateDialogParam(HINSTANCE hInstance, LPTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HWND pxGetDlgItem(HWND hDlg, int nIDDlgItem);
BOOL pxGetMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
BOOL pxIsDialogMessage(HWND hDlg, LPMSG lpMsg);
BOOL pxTranslateMessage(const MSG* lpMsg);
LRESULT pxDispatchMessage(const MSG* lpMsg);
HMODULE pxLoadLibrary(LPCTSTR lpLibFileName);
FARPROC pxGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
HANDLE pxCreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
DWORD pxGetCurrentThreadId();
DWORD pxGetCurrentProcessId();
LPTOP_LEVEL_EXCEPTION_FILTER pxSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
HBITMAP pxCreateDIBSection(HDC hdc, const BITMAPINFO* pbmi, UINT usage, VOID** ppvBits, HANDLE hSection, DWORD offset);
HGDIOBJ pxSelectObject(HDC hdc, HGDIOBJ h);
HDC pxGetDC(HWND hWnd);
int pxReleaseDC(HWND hWnd, HDC hDC);
HGDIOBJ pxGetStockObject(int i);
HWND pxFindWindow(LPCTSTR lpClassName, LPCTSTR lpWindowName);
HWND pxFindWindowEx(HWND hWndParent, HWND hWndChildAfter, LPCTSTR lpszClass, LPCTSTR lpszWindow);
HWND pxSetParent(HWND hWndChild, HWND hWndNewParent);
HDC pxCreateCompatibleDC(HDC hdc);
UINT_PTR pxSetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc);

// none
void pxPostQuitMessage(int nExitCode);
