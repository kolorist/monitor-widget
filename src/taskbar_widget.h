#pragma once

#include <Windows.h>

#include <floral/stdaliases.h>
#include <floral/memory.h>
#include <floral/vector_math.h>

struct ApplicationState;
struct ApplicationConfigs;
struct OSTaskBarState;

enum class Orientation : u8
{
    Landscape = 0,
    Portrait
};

struct OSTaskBarState
{
    HWND hTaskBarWnd;
    HWND hReBarWnd;
    HWND hTaskWnd;
    HWND hTrayNotifyWnd;
};

struct UIWidgetSurfaceState
{
    HDC hdc;
    HBITMAP buffer;
    voidptr bufferData;
    vec2i size;
    s32 dpi;
};

struct UIWidgetState
{
    HWND hwnd;

    vec2i origin;
    s32 landscapeLength;
    s32 portraitLength;
    Orientation orientation;

    // canvas
    s32 dpi;
    vec2i taskbarSize;

    bool ready;
};

// ----------------------------------------------------------------------------

HWND UIWidgetInitialize(const HINSTANCE i_appInstance, linear_allocator_t* const i_allocator);
void UIWidgetReload();
void UIWidgetCleanUp();
bool UIWidgetToggle(const bool i_visible);
void UIWidgetSwitchMode();
