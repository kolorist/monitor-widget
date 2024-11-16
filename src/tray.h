#pragma once

#include "types.h"

#include <floral/stdaliases.h>

struct TrayIconState
{
    bool ready;
};

// ----------------------------------------------------------------------------

bool UITrayIconInitialize(const HINSTANCE i_appInstance, HWND i_parentWnd);
void UITrayIconCleanUp();
void UITrayIconHandleMessage(const u32 i_message, const HWND i_hwnd);
