#pragma once

#include "configs.h"

// according to the C++11 Standard
// for Win32, popular Unix-like systems (Linux, Mac OS X)
// ref: http://en.cppreference.com/w/cpp/language/types

// integer
typedef char c8;
typedef wchar_t c16;
typedef signed char s8;
typedef unsigned char u8;

typedef signed short int s16;
typedef unsigned short int u16;

typedef signed int s32;
typedef unsigned int u32;

#if defined(__LP64__) && !defined(__INTELLISENSE__)
typedef long s64;
typedef unsigned long u64;
#else
typedef signed long long int s64;
typedef unsigned long long int u64;
#endif

// floating point number
typedef float f32;
typedef double f64;

// size type
typedef u32 size32;
typedef u32 usize32;
typedef s32 ssize32;
typedef u64 size64;
typedef u64 usize64;
typedef s64 ssize64;
// index type
typedef u32 uidx32;
typedef s32 sidx32;
typedef u64 uidx64;
typedef s64 sidx64;

#if defined(FLORAL_ARCH_64BIT)
typedef size64 size;
typedef usize64 usize;
typedef ssize64 ssize;
typedef uidx64 uidx;  // unsigned index type
typedef sidx64 sidx;  // signed index type
typedef ssize64 aptr; // pointer type for arithmetic
#elif defined(FLORAL_ARCH_32BIT)
typedef size32 size;
typedef usize32 usize;
typedef ssize32 ssize;
typedef uidx32 uidx;
typedef sidx32 sidx;
typedef size32 aptr;
#else
// ERROR
#endif

// c-string
typedef char* cstr;
typedef wchar_t* wcstr;
typedef const char* const_cstr;
typedef const wchar_t* const_wcstr;
#if defined(UNICODE)
typedef wchar_t tchar;
typedef wcstr tcstr;
typedef const_wcstr const_tcstr;
#else
typedef char tchar;
typedef cstr tcstr;
typedef const_cstr const_tcstr;
#endif

// normal pointer
typedef u8* p8;
typedef u16* p16;
typedef u32* p32;
typedef u64* p64;

// void pointer
typedef void* voidptr;
typedef const void* const_voidptr;

#define ATOMIC_TYPE(type) volatile type
