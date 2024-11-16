#pragma once

#include <Windows.h>
#include <ShellScalingApi.h>
#include <gdiplus.h>

#include <floral/stdaliases.h>

namespace gdiapi
{
// ----------------------------------------------------------------------------

class PrivateFontCollection : public Gdiplus::PrivateFontCollection
{
private:
    using Super = Gdiplus::PrivateFontCollection;

public:
    PrivateFontCollection();

    void AddMemoryFont(const void* memory, INT length);
};

// ----------------------------------------------------------------------------

class Font : public Gdiplus::Font
{
private:
    using Super = Gdiplus::Font;

public:
    Font(const WCHAR* familyName,
         Gdiplus::REAL emSize,
         INT style = Gdiplus::FontStyleRegular,
         Gdiplus::Unit unit = Gdiplus::UnitPoint,
         const Gdiplus::FontCollection* fontCollection = NULL);
    ~Font() = default;
};

// ----------------------------------------------------------------------------

class StringFormat : public Gdiplus::StringFormat
{
private:
    using Super = Gdiplus::StringFormat;

public:
    StringFormat();

    void SetMeasurableCharacterRanges(INT rangeCount, const Gdiplus::CharacterRange* ranges);
};

// ----------------------------------------------------------------------------

class Region : public Gdiplus::Region
{
private:
    using Super = Gdiplus::Region;

public:
    Region();
    void GetBounds(Gdiplus::RectF* rect, const Gdiplus::Graphics* g) const;
};

// ----------------------------------------------------------------------------

class SolidBrush : public Gdiplus::SolidBrush
{
private:
    using Super = Gdiplus::SolidBrush;

public:
    SolidBrush(const Gdiplus::Color& color);
};

// ----------------------------------------------------------------------------

class Graphics : public Gdiplus::Graphics
{
private:
    using Super = Gdiplus::Graphics;

public:
    Graphics(HDC hdc);
    ~Graphics() = default;

    void Clear(const Gdiplus::Color& color);
    void SetCompositingQuality(Gdiplus::CompositingQuality compositingQuality);
    void SetCompositingMode(Gdiplus::CompositingMode compositingMode);
    void SetTextRenderingHint(Gdiplus::TextRenderingHint newMode);
    void SetSmoothingMode(Gdiplus::SmoothingMode smoothingMode);
    void MeasureCharacterRanges(const WCHAR* string,
                                INT length,
                                const Gdiplus::Font* font,
                                const Gdiplus::RectF& layoutRect,
                                const StringFormat* stringFormat,
                                INT regionCount,
                                Gdiplus::Region* regions) const;
    void DrawString(const WCHAR* string,
                    INT length,
                    const Gdiplus::Font* font,
                    const Gdiplus::PointF& origin,
                    const Gdiplus::Brush* brush);
    void DrawString(const WCHAR* string,
                    INT length,
                    const Gdiplus::Font* font,
                    const Gdiplus::RectF& layoutRect,
                    const Gdiplus::StringFormat* stringFormat,
                    const Gdiplus::Brush* brush);
};

// ----------------------------------------------------------------------------

void GdiplusStartup(ULONG_PTR* token, const Gdiplus::GdiplusStartupInput* input, Gdiplus::GdiplusStartupOutput* output);
void GdiplusShutdown(ULONG_PTR token);

// ----------------------------------------------------------------------------
} // namespace gdiapi
