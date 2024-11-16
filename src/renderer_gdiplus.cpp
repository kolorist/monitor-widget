#include "renderer_gdiplus.h"

#include <floral/log.h>
#include <floral/time.h>

#include <new>
#include <resource.h>

#include "gdiapi.h"
#include "utils.h"
#include "scripting.h"

void DrawHTMLString(RNDState* i_gdiState, const gdiapi::Font* const i_font,
                    const Gdiplus::RectF& i_rect, HTMLText* const i_text,
                    const s32 i_textAlign, const s32 i_lineAlign,
                    arena_t* const i_arena);

// ----------------------------------------------------------------------------

static s32 ScriptingDrawRect(lua_State* i_vm)
{
    RNDState* const gdiState = (RNDState* const)lua_touserdata(i_vm, lua_upvalueindex(1));
    const s32 nArgs = lua_gettop(i_vm);
    FLORAL_ASSERT(nArgs == 6);
    s32 x = (s32)lua_tointeger(i_vm, 1);
    s32 y = (s32)lua_tointeger(i_vm, 2);
    s32 w = (s32)lua_tointeger(i_vm, 3);
    s32 h = (s32)lua_tointeger(i_vm, 4);
    const u32 color = (u32)lua_tointeger(i_vm, 5);
    f32 width = (f32)lua_tonumber(i_vm, 6);

    const f32 dpiScale = (f32)gdiState->dpiScale;
    const f32 fx = (f32)x * dpiScale;
    const f32 fy = (f32)y * dpiScale;
    const f32 fw = (f32)w * dpiScale;
    const f32 fh = (f32)h * dpiScale;
    width = width * dpiScale;

    Gdiplus::Pen pen(Gdiplus::Color(BYTE(color & 0xff),
                                    BYTE((color & 0xff000000) >> 24),
                                    BYTE((color & 0xff0000) >> 16),
                                    BYTE((color & 0xff00) >> 8)),
                     width);
    gdiState->graphics->DrawRectangle(&pen, fx, fy, fw, fh);

    return 0;
}

static s32 ScriptingDrawArc(lua_State* i_vm)
{
    RNDState* const gdiState = (RNDState* const)lua_touserdata(i_vm, lua_upvalueindex(1));
    const s32 nArgs = lua_gettop(i_vm);
    FLORAL_ASSERT(nArgs == 8);
    f32 x = (f32)lua_tonumber(i_vm, 1);
    f32 y = (f32)lua_tonumber(i_vm, 2);
    f32 w = (f32)lua_tonumber(i_vm, 3);
    f32 h = (f32)lua_tonumber(i_vm, 4);
    const u32 color = (u32)lua_tointeger(i_vm, 5);
    f32 width = (f32)lua_tonumber(i_vm, 6);
    f32 start = (f32)lua_tonumber(i_vm, 7);
    f32 sweep = (f32)lua_tonumber(i_vm, 8);

    x *= gdiState->dpiScale;
    y *= gdiState->dpiScale;
    w *= gdiState->dpiScale;
    h *= gdiState->dpiScale;
    width *= gdiState->dpiScale;
    const f32 halfWidth = width * 0.5f;

    if (gdiState->debugDrawLayout)
    {
        Gdiplus::Pen pen(Gdiplus::Color::Red, 1);
        gdiState->graphics->DrawRectangle(&pen, x, y, w, h);
    }

    Gdiplus::Pen pen(Gdiplus::Color(BYTE(color & 0xff),
                                    BYTE((color & 0xff000000) >> 24),
                                    BYTE((color & 0xff0000) >> 16),
                                    BYTE((color & 0xff00) >> 8)),
                     width);
    pen.SetAlignment(Gdiplus::PenAlignment::PenAlignmentCenter);
    // workaround because PenAlignmentInset doesn't work with DrawArc
    gdiState->graphics->DrawArc(&pen, x + halfWidth, y + halfWidth, w - width, h - width, start, sweep);
    return 0;
}

static s32 ScriptingFillRect(lua_State* i_vm)
{
    RNDState* const gdiState = (RNDState* const)lua_touserdata(i_vm, lua_upvalueindex(1));
    const s32 nArgs = lua_gettop(i_vm);
    FLORAL_ASSERT(nArgs == 5);
    s32 x = (s32)lua_tointeger(i_vm, 1);
    s32 y = (s32)lua_tointeger(i_vm, 2);
    s32 w = (s32)lua_tointeger(i_vm, 3);
    s32 h = (s32)lua_tointeger(i_vm, 4);
    const u32 color = (u32)lua_tointeger(i_vm, 5);

    const f32 dpiScale = (f32)gdiState->dpiScale;
    const f32 fx = (f32)x * dpiScale;
    const f32 fy = (f32)y * dpiScale;
    const f32 fw = (f32)w * dpiScale;
    const f32 fh = (f32)h * dpiScale;

    Gdiplus::SolidBrush brush(Gdiplus::Color(BYTE(color & 0xff),
                                             BYTE((color & 0xff000000) >> 24),
                                             BYTE((color & 0xff0000) >> 16),
                                             BYTE((color & 0xff00) >> 8)));
    gdiState->graphics->FillRectangle(&brush, fx, fy, fw, fh);

    return 0;
}

static s32 ScriptingLoadFont(lua_State* i_vm)
{
    LOG_SCOPE(font);
    RNDState* const gdiState = (RNDState* const)lua_touserdata(i_vm, lua_upvalueindex(1));

    const s32 nArgs = lua_gettop(i_vm);
    FLORAL_ASSERT(nArgs == 2);
    const str16 fontFamily = str16_duplicate(&gdiState->arena, lua_tostring(i_vm, 1));
    const f32 fontSize = (f32)lua_tointeger(i_vm, 2);
    const f32 dpiScale = (f32)gdiState->dpiScale;

    const gdiapi::PrivateFontCollection* fontsCollection = gdiState->fontsCollection;
    ssize fontStyleHandle = handle_pool_alloc(&gdiState->fontStyleHandlesPool);
    gdiState->fontDesc[fontStyleHandle].fontFamily = fontFamily;
    gdiState->fontDesc[fontStyleHandle].size = fontSize;
    ::new (&gdiState->fontStylesPool[fontStyleHandle]) gdiapi::Font(
        fontFamily.data, fontSize * dpiScale, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel, fontsCollection);

    LOG_DEBUG(LITERAL("Loaded new font. Family: %s. Size: %.2f (%.2f). Handle: %d"),
              fontFamily.data, fontSize, fontSize * dpiScale, fontStyleHandle);
    lua_pushinteger(i_vm, fontStyleHandle);

    return 1;
}

static s32 ScriptingDrawText(lua_State* i_vm)
{
    RNDState* const gdiState = (RNDState* const)lua_touserdata(i_vm, lua_upvalueindex(1));
    gdiapi::Graphics* g = gdiState->graphics;

    scratch_region_t scratch = scratch_begin(&gdiState->arena);

    const s32 nArgs = lua_gettop(i_vm);
    FLORAL_ASSERT(nArgs == 8);

    const s32 fontStyleHandle = (s32)lua_tointeger(i_vm, 1);
    const s32 x = (s32)lua_tointeger(i_vm, 2);
    const s32 y = (s32)lua_tointeger(i_vm, 3);
    const s32 w = (s32)lua_tointeger(i_vm, 4);
    const s32 h = (s32)lua_tointeger(i_vm, 5);
    const_cstr str = lua_tostring(i_vm, 6);
    const size strLen = cstr_length(str);
    const s32 textAlign = (s32)lua_tointeger(i_vm, 7);
    const s32 lineAlign = (s32)lua_tointeger(i_vm, 8);
    const f32 dpiScale = (f32)gdiState->dpiScale;

    FLORAL_ASSERT(fontStyleHandle >= 0);

    const s32 tstrLen = MultiByteToWideChar(CP_UTF8, 0, str, strLen, NULL, 0) + 1;
    const tcstr tstr = (tcstr)arena_push_podarr(scratch.arena, tchar, tstrLen);
    const s32 written = MultiByteToWideChar(CP_UTF8, 0, str, strLen, tstr, tstrLen);
    tstr[written] = 0;

    HTMLText textLine = HTMLParse(tstr, tstrLen, scratch.arena);
    const gdiapi::Font* font = &gdiState->fontStylesPool[fontStyleHandle];
    const Gdiplus::RectF textRect((f32)x * dpiScale, (f32)y * dpiScale, (f32)w * dpiScale, (f32)h * dpiScale);
    DrawHTMLString(gdiState, font, textRect, &textLine, textAlign, lineAlign, scratch.arena);

    scratch_end(&scratch);
    return 0;
}

static s32 ScriptingDebugSetLayoutDraw(lua_State* i_vm)
{
    RNDState* const gdiState = (RNDState* const)lua_touserdata(i_vm, lua_upvalueindex(1));
    const s32 nArgs = lua_gettop(i_vm);
    FLORAL_ASSERT(nArgs == 1);
    gdiState->debugDrawLayout = (bool)lua_toboolean(i_vm, 1);
    return 0;
}

// ----------------------------------------------------------------------------

void DrawHTMLString(RNDState* i_gdiState, const gdiapi::Font* const i_font,
                    const Gdiplus::RectF& i_rect, HTMLText* const i_text,
                    const s32 i_textAlign, const s32 i_lineAlign,
                    arena_t* const i_arena)
{
    gdiapi::Graphics& g = *i_gdiState->graphics;
    scratch_region_t scratch = scratch_begin(i_arena);

    const_tcstr rawText = i_text->rawData;
    const s32 rawTextLen = (s32)i_text->rawLength;

    array_t<Gdiplus::CharacterRange> ranges = arena_create_array(scratch.arena,
                                                                 Gdiplus::CharacterRange,
                                                                 i_text->partsCount);
    dll_t<HTMLTextPart>::node_t* it = nullptr;
    dll_for_each(&i_text->parts, it)
    {
        const HTMLTextPart& part = it->data;
        array_push_back(&ranges, Gdiplus::CharacterRange((s32)part.startIndex, (s32)part.length));
    }
    gdiapi::StringFormat strFormat = {};
    strFormat.SetAlignment(Gdiplus::StringAlignment(i_textAlign));
    strFormat.SetLineAlignment(Gdiplus::StringAlignment(i_lineAlign));
    strFormat.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoFitBlackBox |
                             Gdiplus::StringFormatFlags::StringFormatFlagsMeasureTrailingSpaces |
                             Gdiplus::StringFormatFlags::StringFormatFlagsNoClip);
    strFormat.SetMeasurableCharacterRanges((s32)ranges.size, ranges.data);
    const s32 numRegions = strFormat.GetMeasurableCharacterRangeCount();
    gdiapi::Region* regions = arena_push_objarr(scratch.arena, gdiapi::Region, numRegions);
    g.MeasureCharacterRanges(rawText, rawTextLen, i_font, i_rect, &strFormat, numRegions, regions);

    // actual size of the string (including the small padding spaces around it)
    Gdiplus::RectF stringRect;
    g.MeasureString(rawText, rawTextLen, i_font, i_rect, &strFormat, &stringRect);

    if (i_gdiState->debugDrawLayout)
    {
        Gdiplus::Pen pen(Gdiplus::Color::Red, 1);
        g.DrawRectangle(&pen, i_rect);
    }

    s32 regionIdx = 0;
    f32 x = stringRect.X;
    dll_for_each(&i_text->parts, it)
    {
        const HTMLTextPart& part = it->data;
        const_wcstr partText = &rawText[part.startIndex];
        const s32 partTextLen = (s32)part.length;

        Gdiplus::RectF rect;
        regions[regionIdx].GetBounds(&rect, &g);

        gdiapi::SolidBrush brush(Gdiplus::Color(BYTE(0xff),                            // Alpha
                                                BYTE((part.color & 0xff)),             // Red
                                                BYTE((part.color & 0xff00) >> 8),      // Green
                                                BYTE((part.color & 0xff0000) >> 16))); // Blue
        g.DrawString(partText, partTextLen, i_font, Gdiplus::PointF(x, rect.Y), &brush);
        regionIdx++;
        x += rect.Width;
    }

    for (ssize i = 0; i < numRegions; i++)
    {
        regions[i].~Region();
    }

    scratch_end(&scratch);
}

static bool AddFontData(RNDState* const io_gdiState, const_voidptr i_data, const size i_size)
{
    RNDState& state = *io_gdiState;

    state.fontsCollection->AddMemoryFont(i_data, (s32)i_size);
    return true;
}

// ----------------------------------------------------------------------------

bool RNDInitialize(RNDState* const io_gdiState, HINSTANCE i_appInstance, HDC i_hdc, linear_allocator_t* const i_allocator)
{
    LOG_SCOPE(gdi);
    MARK_UNUSED(i_hdc);
    RNDState& state = *io_gdiState;
    state.arena = create_arena(i_allocator, SIZE_MB(2));
    arena_t* const arena = &state.arena;

    Gdiplus::GdiplusStartupInput startupInput = {};
    startupInput.GdiplusVersion = 1;
    gdiapi::GdiplusStartup(&state.token, &startupInput, nullptr);

    state.fontsCollection = arena_push_obj(arena, gdiapi::PrivateFontCollection);
    state.fontStyleHandlesPool = arena_create_handle_pool(arena, ssize, 128);
    state.fontStylesPool = arena_create_array(arena, gdiapi::Font, state.fontStyleHandlesPool.capacity);
    state.fontStylesPool.size = state.fontStylesPool.capacity;
    state.fontDesc = arena_create_array(arena, FontDescription, state.fontStyleHandlesPool.capacity);
    state.fontDesc.size = state.fontStylesPool.capacity;

    LOG_DEBUG("Fonts pool created with capacity of %d", state.fontStyleHandlesPool.capacity);

    // load 2 embedded fonts
    voidptr fontData = nullptr;
    size fontDataSize = 0;
    if (!UTLLoadEmbeddedData(i_appInstance, IDF_CHARFONT, &fontData, &fontDataSize))
    {
        UTLShowMessage(NULL, UTLSeverity::Error, LITERAL("Failed to load embedded font data (IDF_CHARFONT)."));
        return false;
    }
    if (!AddFontData(&state, fontData, fontDataSize))
    {
        UTLShowMessage(NULL, UTLSeverity::Error, LITERAL("Failed to register font data with GDI+."));
        return false;
    }

    if (!UTLLoadEmbeddedData(i_appInstance, IDF_ICONFONT, &fontData, &fontDataSize))
    {
        UTLShowMessage(NULL, UTLSeverity::Error, LITERAL("Failed to load embedded font data (IDF_CHARFONT)."));
        return false;
    }
    if (!AddFontData(&state, fontData, fontDataSize))
    {
        UTLShowMessage(NULL, UTLSeverity::Error, LITERAL("Failed to register font data with GDI+."));
        return false;
    }

    state.graphics = arena_push_pod(arena, gdiapi::Graphics);
    state.graphicsReady = false;
    return true;
}

void RNDBindScriptingAPIs(RNDState* const i_gdiState)
{
    RNDState& state = *i_gdiState;
    SCRRegisterFunc(&ScriptingDrawRect, "draw_rect", &state);
    SCRRegisterFunc(&ScriptingDrawArc, "draw_arc", &state);
    SCRRegisterFunc(&ScriptingFillRect, "fill_rect", &state);
    SCRRegisterFunc(&ScriptingLoadFont, "load_font", &state);
    SCRRegisterFunc(&ScriptingDrawText, "draw_text", &state);

    SCRRegisterFunc(&ScriptingDebugSetLayoutDraw, "debug_set_layout_draw", &state);
}

void RNDDestroyAllResources(RNDState* i_gdiState)
{
    handle_pool_t<ssize>* const fontStyleHandlesPool = &i_gdiState->fontStyleHandlesPool;
    array_t<gdiapi::Font>* const fontStylesPool = &i_gdiState->fontStylesPool;
    for (size i = 0; i < fontStyleHandlesPool->count; i++)
    {
        ssize handle = fontStyleHandlesPool->dense[i];
        (*fontStylesPool)[handle].~Font();
    }
    handle_pool_reset(fontStyleHandlesPool);
}

void RNDRefresh(RNDState* i_gdiState, HDC i_hdc, const vec2i& i_resolution, const f32 i_dpiScale)
{
    LOG_SCOPE(gdi);
    RNDState& state = *i_gdiState;

    if (state.graphicsReady)
    {
        state.graphics->~Graphics();
        state.graphicsReady = false;
    }

    if (i_resolution.x <= 0 || i_resolution.y <= 0)
    {
        LOG_WARNING("Cannot create GDI+ Graphics context due to invalid buffer size: %d x %d",
                    i_resolution.x, i_resolution.y);
        return;
    }

    state.graphics = ::new (state.graphics) gdiapi::Graphics(i_hdc);
    state.graphicsReady = true;

    gdiapi::Graphics& g = *state.graphics;

    g.Clear(0);
    g.SetCompositingQuality(Gdiplus::CompositingQuality::CompositingQualityAssumeLinear);
    g.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceOver);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAliasGridFit);
    g.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);

    g.Flush(Gdiplus::FlushIntention::FlushIntentionSync);

    state.lastRenderTimePoint = time_get_absolute_highres_ms();
    state.lastRenderTime = 0.0f;
    state.deltaRenderTime = 0.0f;
    state.resolution = i_resolution;

    // refresh fonts if the dpi is updated
    if (state.dpiScale != i_dpiScale)
    {
        handle_pool_t<ssize>* const fontStyleHandlesPool = &i_gdiState->fontStyleHandlesPool;
        array_t<gdiapi::Font>* const fontStylesPool = &i_gdiState->fontStylesPool;
        for (size i = 0; i < fontStyleHandlesPool->count; i++)
        {
            ssize handle = fontStyleHandlesPool->dense[i];
            (*fontStylesPool)[handle].~Font();

            const FontDescription& fontDesc = i_gdiState->fontDesc[handle];
            ::new (&i_gdiState->fontStylesPool[handle]) gdiapi::Font(
                fontDesc.fontFamily.data, fontDesc.size * i_dpiScale,
                Gdiplus::FontStyleRegular, Gdiplus::UnitPixel, i_gdiState->fontsCollection);

            LOG_VERBOSE(LITERAL("DPI changed. Reloaded font: %s. Size: %.2f (%.2f)"),
                        fontDesc.fontFamily.data, fontDesc.size, fontDesc.size * i_dpiScale);
        }
        state.dpiScale = i_dpiScale;
    }

    LOG_DEBUG("RenderSurface updated. Dimension: %d x %d. DPI Scale: %.2f", i_resolution.x, i_resolution.y, i_dpiScale);
}

void RNDCleanUp(RNDState* const i_gdiState)
{
    RNDState& state = *i_gdiState;
    gdiapi::GdiplusShutdown(state.token);
}

bool RNDBeginRender(RNDState* const i_gdiState)
{
    if (!i_gdiState->graphicsReady)
    {
        return false;
    }

    i_gdiState->graphics->Clear(0);

    if (i_gdiState->debugDrawLayout)
    {
        Gdiplus::Pen pen(Gdiplus::Color::Green, 1);
        pen.SetAlignment(Gdiplus::PenAlignment::PenAlignmentInset);
        i_gdiState->graphics->DrawRectangle(&pen, Gdiplus::Rect(0, 0, i_gdiState->resolution.x - 1, i_gdiState->resolution.y - 1));
    }

    return true;
}

void RNDEndRender(RNDState* const i_gdiState)
{
    FLORAL_ASSERT(i_gdiState->graphicsReady);
    i_gdiState->graphics->Flush(Gdiplus::FlushIntention::FlushIntentionSync);
}
