#pragma once

#include "configs.h"
#include "stdaliases.h"

#include <stdio.h>
#include <stdlib.h>

// ----------------------------------------------------------------------------

enum class assert_action_e
{
    abort = 0,
    debug_break,
    ignore
};

assert_action_e assertion_report(const_cstr expr, const_cstr file, const u32 line);
assert_action_e assertion_report_msg(const_cstr expr, const_cstr msg, const_cstr file, const u32 line);
assert_action_e assertion_report_msgonly(const_cstr msg, const_cstr file, const u32 line);
assert_action_e assertion_report_dlg(const_cstr title, const_cstr msg, const_cstr file, const u32 line);

#define FLORAL_CRASH                   \
    {                                  \
        volatile u32* crash = nullptr; \
        *crash = 10;                   \
    }

#if defined(FLORAL_PLATFORM_WINDOWS)
#  define FLORAL_DEBUG_BREAK __debugbreak
#  define FLORAL_ABORT abort()
#else
#  define FLORAL_DEBUG_BREAK __builtin_trap
#endif

#if !defined(NDEBUG)
#  if defined(FLORAL_PLATFORM_WINDOWS) || defined(FLORAL_PLATFORM_ANDROID)
#    define FLORAL_ASSERT(x)                                      \
        do                                                        \
        {                                                         \
            if (!(x))                                             \
            {                                                     \
                switch (assertion_report(#x, __FILE__, __LINE__)) \
                {                                                 \
                case assert_action_e::debug_break:                \
                    FLORAL_DEBUG_BREAK();                         \
                    break;                                        \
                case assert_action_e::abort:                      \
                    exit(1);                                      \
                    break;                                        \
                case assert_action_e::ignore:                     \
                default:                                          \
                    break;                                        \
                }                                                 \
            }                                                     \
        } while (0)

#    define FLORAL_ASSERT_MSG(x, msg)                                      \
        do                                                                 \
        {                                                                  \
            if (!(x))                                                      \
            {                                                              \
                switch (assertion_report_msg(#x, msg, __FILE__, __LINE__)) \
                {                                                          \
                case assert_action_e::debug_break:                         \
                    FLORAL_DEBUG_BREAK();                                  \
                    break;                                                 \
                case assert_action_e::abort:                               \
                    exit(1);                                               \
                    break;                                                 \
                case assert_action_e::ignore:                              \
                default:                                                   \
                    break;                                                 \
                }                                                          \
            }                                                              \
        } while (0)

#    define FLORAL_ASSERT_MSG_ONLY(x, msg)                                 \
        do                                                                 \
        {                                                                  \
            if (!(x))                                                      \
            {                                                              \
                switch (assertion_report_msgonly(msg, __FILE__, __LINE__)) \
                {                                                          \
                case assert_action_e::debug_break:                         \
                    FLORAL_DEBUG_BREAK();                                  \
                    break;                                                 \
                case assert_action_e::abort:                               \
                    exit(1);                                               \
                    break;                                                 \
                case assert_action_e::ignore:                              \
                default:                                                   \
                    break;                                                 \
                }                                                          \
            }                                                              \
        } while (0)
#  else
#    define FLORAL_ASSERT(x) \
        do                   \
        {                    \
            (void)sizeof(x); \
        } while (0)

#    define FLORAL_ASSERT_MSG(x, msg) \
        do                            \
        {                             \
            (void)sizeof(x);          \
        } while (0)

#    define FLORAL_ASSERT_MSG_ONLY(x, msg) \
        do                                 \
        {                                  \
            (void)sizeof(x);               \
        } while (0)
#  endif
#else // NDEBUG
#  define FLORAL_ASSERT(x) \
      do                   \
      {                    \
          (void)sizeof(x); \
      } while (0)

#  define FLORAL_ASSERT_MSG(x, msg) \
      do                            \
      {                             \
          (void)sizeof(x);          \
      } while (0)

#  define FLORAL_ASSERT_MSG_ONLY(x, msg) \
      do                                 \
      {                                  \
          (void)sizeof(x);               \
      } while (0)
#endif
