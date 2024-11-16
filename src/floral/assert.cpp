#include "assert.h"

#include "string_utils.h"

#if defined(FLORAL_PLATFORM_WINDOWS)
#  include <Windows.h>
#elif defined(FLORAL_PLATFORM_ANDROID)
#  include <android/log.h>
#else
#endif

// ----------------------------------------------------------------------------

assert_action_e assertion_report(const_cstr expr, const_cstr file, const u32 line)
{
    return assertion_report_dlg(expr, "", file, line);
}

assert_action_e assertion_report_msg(const_cstr expr, const_cstr msg, const_cstr file, const u32 line)
{
    return assertion_report_dlg(expr, msg, file, line);
}

assert_action_e assertion_report_msgonly(const_cstr msg, const_cstr file, const u32 line)
{
    return assertion_report_dlg("", msg, file, line);
}

assert_action_e assertion_report_dlg(const_cstr title, const_cstr msg, const_cstr file, const u32 line)
{
#if defined(FLORAL_PLATFORM_WINDOWS)
    c8 errorStr[1024];
    cstr_snprintf(errorStr, 1024, ">> expression: %s\n>> message: %s\n>> location: %s:%d\n", title, msg, file, line);
    OutputDebugStringA(errorStr);
#elif defined(FLORAL_PLATFORM_ANDROID)
    c8 errorStr[1024];
    sprintf(errorStr, 1024, ">> expression: %s\n>> message: %s\n>> location: %s:%d\n", title, msg, file, line);
    __android_log_assert(title, "floral", "%s", errorStr);
#else
    // TODO
#endif
    return assert_action_e::debug_break;
}
