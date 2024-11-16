#pragma once

#include <Windows.h>

#include <floral/stdaliases.h>
#include <floral/container.h>

struct linear_allocator_t;

struct MainDialogState
{
    HINSTANCE hInstance;

    HWND hWnd;
    HFONT hFont;
    UINT wmTaskBarRestart;

    HWND lblAllocsCount;
    HWND lblFreesCount;
    HWND lblUsedMemory;

    inplace_array_t<HWND, 2> hWnds;

    linear_allocator_t allocator;
};

HWND UIMainDialogCreate(HINSTANCE i_hInstance, linear_allocator_t* const i_allocator);
void UIMainDialogRun();
void UIMainDialogCleanUp();
