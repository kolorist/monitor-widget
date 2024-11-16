#pragma once

// operating system platform
#if defined(_WIN64) || defined(_WIN32)
#  define FLORAL_PLATFORM_WINDOWS
#  define FLORAL_PLATFORM_HAS_DX12
#  define FLORAL_PLATFORM_HAS_GL
#  define FLORAL_PLATFORM_HAS_VULKAN
#elif defined(__ANDROID__) || defined(__linux__)
#  define FLORAL_PLATFORM_LINUX
#  define FLORAL_PLATFORM_HAS_VULKAN
#  if defined(__ANDROID__)
#    define FLORAL_PLATFORM_ANDROID
#    define FLORAL_PLATFORM_HAS_GLES
#  else
#    define FLORAL_PLATFORM_HAS_GL
#  endif
#else
// TODO
#endif

// cpu
#if defined(__aarch64__) || defined(__arm__)
#  define FLORAL_CPU_ARM
#elif defined(__x86_64__) || defined(__i386__)
#  define FLORAL_CPU_INTEL
#else
// TODO
#endif

// cpu architecture
#if defined(__x86_64__) || defined(__aarch64__) || defined(_M_X64)
#  define FLORAL_ARCH_64BIT
#elif defined(__i386__) || defined(__arm__) || defined(_M_IX86)
#  define FLORAL_ARCH_32BIT
#else
// TODO
#endif

#if !defined(FLORAL_MAX_LOCAL_BUFFER_LENGTH)
#  define FLORAL_MAX_LOCAL_BUFFER_LENGTH 1024
#endif

#if !defined(FLORAL_MAX_PATH_LENGTH)
#  define FLORAL_MAX_PATH_LENGTH 512
#endif

#if !defined(FLORAL_MAX_NAME_LENGTH)
#  define FLORAL_MAX_NAME_LENGTH 32
#endif

#if !defined(FLORAL_MAX_PATH_DEPTH)
#  define FLORAL_MAX_PATH_DEPTH 16
#endif

// ----------------------------------------------------------------------------
// memory
// ----------------------------------------------------------------------------

// alignment of a memory allocation buffer for underlying OS's allocator (usually big, in KBytes)
#ifndef MEMORY_DEFAULT_MALLOC_ALIGNMENT
#  define MEMORY_DEFAULT_MALLOC_ALIGNMENT (SIZE_KB(4))
#endif

// alignment of a single allocated memory region in the allocator (usually small, in Bytes)
#ifndef MEMORY_DEFAULT_ALIGNMENT
#  define MEMORY_DEFAULT_ALIGNMENT 8
#endif

#ifndef MEMORY_SIMD_ALIGNMENT
#  define MEMORY_SIMD_ALIGNMENT 32
#endif

#if !defined(LOG_MAX_SCOPES)
#  define LOG_MAX_SCOPES 16
#endif
#define LOG_MAX_LOGGERS 4

#if !defined(MATH_PI)
#  define MATH_PI 3.1415926525898f
#endif

#if !defined(MATH_HALF_PI)
#  define MATH_HALF_PI 1.5707963267948966f
#endif
