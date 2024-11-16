#pragma once

#include <floral/container.h>
#include <floral/stdaliases.h>
#include <floral/string_utils.h>

///////////////////////////////////////////////////////////////////////////////
// Simple HTML tag parser

struct HTMLTextPart
{
    s32 startIndex;
    s32 length;
    u32 color;
};

struct HTMLText
{
    tcstr rawData;
    size rawLength;
    size rawCapacity;

    dll_t<HTMLTextPart> parts;
    u32 partsCount;
};

struct HTMLElement
{
    tstr tag;
    tstr value;
    tstr inner;
};

HTMLText HTMLParse(const_tcstr i_str, const size i_strLen, arena_t* const i_arena);

///////////////////////////////////////////////////////////////////////////////
// Utilities

bool OSGetStartOnBoot();
void OSSetStartOnBoot(const bool i_enabled);
s32 OSGetDPI(HWND i_hwnd);

bool UTLLoadEmbeddedData(HINSTANCE i_appInstance, const u32 i_id, voidptr* o_resource, size* o_len);
enum UTLSeverity : u8
{
    Debug = 0,
    Warning,
    Error
};
void UTLShowMessage(HWND i_hWnd, UTLSeverity i_severity, const_tcstr i_fmt, ...);

