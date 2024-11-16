#pragma once

#include <Windows.h>

#include <floral/stdaliases.h>
#include <floral/memory.h>
#include <floral/container.h>
#include <floral/vector_math.h>
#include <floral/string_utils.h>

namespace gdiapi
{
class Graphics;
class PrivateFontCollection;
class Font;
} // namespace gdiapi

struct FontDescription
{
    str16 fontFamily;
    f32 size;
};

struct RNDState
{
    ULONG_PTR token;

    gdiapi::PrivateFontCollection* fontsCollection;
    handle_pool_t<ssize> fontStyleHandlesPool;
    array_t<gdiapi::Font> fontStylesPool;
    array_t<FontDescription> fontDesc;

    gdiapi::Graphics* graphics;
    bool graphicsReady;
    f32 dpiScale;
    vec2i resolution;

    f64 lastRenderTimePoint;
    f32 lastRenderTime;
    f32 deltaRenderTime;

    // debug
    bool debugDrawLayout : 1;

    arena_t arena;
};

bool RNDInitialize(RNDState* const io_gdiState, HINSTANCE i_appInstance, HDC i_hdc, linear_allocator_t* const i_allocator);
void RNDBindScriptingAPIs(RNDState* const i_gdiState);
void RNDDestroyAllResources(RNDState* i_gdiState);
void RNDRefresh(RNDState* i_gdiState, HDC i_hdc, const vec2i& i_resolution, const f32 i_dpiScale);
void RNDCleanUp(RNDState* const i_gdiState);
bool RNDBeginRender(RNDState* const i_gdiState);
void RNDEndRender(RNDState* const i_gdiState);
