#include "gdiapi.h"

#include <floral/assert.h>
#include <floral/log.h>

namespace gdiapi
{
// ----------------------------------------------------------------------------

void AssertGdiplusStatusOk(const Gdiplus::Status i_status)
{
    if (i_status != Gdiplus::Status::Ok)
    {
        LOG_ERROR("Error with GDI+. Code: %d", (s32)i_status);
        FLORAL_DEBUG_BREAK();
    }
}

// ----------------------------------------------------------------------------

PrivateFontCollection::PrivateFontCollection()
    : Super()
{
    AssertGdiplusStatusOk(GetLastStatus());
}

void PrivateFontCollection::AddMemoryFont(const void* memory, INT length)
{
    Gdiplus::Status status = Super::AddMemoryFont(memory, length);
    AssertGdiplusStatusOk(status);
}

// ----------------------------------------------------------------------------

Font::Font(const WCHAR* familyName,
           Gdiplus::REAL emSize,
           INT style,
           Gdiplus::Unit unit,
           const Gdiplus::FontCollection* fontCollection)
    : Super(familyName, emSize, style, unit, fontCollection)
{
    AssertGdiplusStatusOk(GetLastStatus());
}

// ----------------------------------------------------------------------------

StringFormat::StringFormat()
    : Super()
{
    AssertGdiplusStatusOk(GetLastStatus());
}

void StringFormat::SetMeasurableCharacterRanges(INT rangeCount, const Gdiplus::CharacterRange* ranges)
{
    Gdiplus::Status status = Super::SetMeasurableCharacterRanges(rangeCount, ranges);
    AssertGdiplusStatusOk(status);
}

// ----------------------------------------------------------------------------

Region::Region()
    : Super()
{
    AssertGdiplusStatusOk(GetLastStatus());
}

void Region::GetBounds(Gdiplus::RectF* rect, const Gdiplus::Graphics* g) const
{
    Gdiplus::Status status = Super::GetBounds(rect, g);
    AssertGdiplusStatusOk(status);
}

// ----------------------------------------------------------------------------

SolidBrush::SolidBrush(const Gdiplus::Color& color)
    : Super(color)
{
    AssertGdiplusStatusOk(GetLastStatus());
}

// ----------------------------------------------------------------------------

Graphics::Graphics(HDC hdc)
    : Super(hdc)
{
    AssertGdiplusStatusOk(GetLastStatus());
}

void Graphics::Clear(const Gdiplus::Color& color)
{
    Gdiplus::Status status = Super::Clear(color);
    AssertGdiplusStatusOk(status);
}

void Graphics::SetCompositingQuality(Gdiplus::CompositingQuality compositingQuality)
{
    Gdiplus::Status status = Super::SetCompositingQuality(compositingQuality);
    AssertGdiplusStatusOk(status);
}

void Graphics::SetCompositingMode(Gdiplus::CompositingMode compositingMode)
{
    Gdiplus::Status status = Super::SetCompositingMode(compositingMode);
    AssertGdiplusStatusOk(status);
}

void Graphics::SetTextRenderingHint(Gdiplus::TextRenderingHint newMode)
{
    Gdiplus::Status status = Super::SetTextRenderingHint(newMode);
    AssertGdiplusStatusOk(status);
}

void Graphics::SetSmoothingMode(Gdiplus::SmoothingMode smoothingMode)
{
    Gdiplus::Status status = Super::SetSmoothingMode(smoothingMode);
    AssertGdiplusStatusOk(status);
}

void Graphics::MeasureCharacterRanges(const WCHAR* string,
                                      INT length,
                                      const Gdiplus::Font* font,
                                      const Gdiplus::RectF& layoutRect,
                                      const StringFormat* stringFormat,
                                      INT regionCount,
                                      Gdiplus::Region* regions) const
{
    Gdiplus::Status status = Super::MeasureCharacterRanges(string, length, font, layoutRect, stringFormat, regionCount, regions);
    AssertGdiplusStatusOk(status);
}

void Graphics::DrawString(const WCHAR* string,
                          INT length,
                          const Gdiplus::Font* font,
                          const Gdiplus::PointF& origin,
                          const Gdiplus::Brush* brush)
{
    Gdiplus::Status status = Super::DrawString(string, length, font, origin, brush);
    AssertGdiplusStatusOk(status);
}

void Graphics::DrawString(const WCHAR* string,
                          INT length,
                          const Gdiplus::Font* font,
                          const Gdiplus::RectF& layoutRect,
                          const Gdiplus::StringFormat* stringFormat,
                          const Gdiplus::Brush* brush)
{
    Gdiplus::Status status = Super::DrawString(string, length, font, layoutRect, stringFormat, brush);
    AssertGdiplusStatusOk(status);
}

// ----------------------------------------------------------------------------

void GdiplusStartup(ULONG_PTR* token, const Gdiplus::GdiplusStartupInput* input, Gdiplus::GdiplusStartupOutput* output)
{
    Gdiplus::Status status = Gdiplus::GdiplusStartup(token, input, output);
    AssertGdiplusStatusOk(status);
}

void GdiplusShutdown(ULONG_PTR token)
{
    Gdiplus::GdiplusShutdown(token);
}

// ----------------------------------------------------------------------------
} // namespace gdiapi
