#include "thread.h"

#if defined(FLORAL_PLATFORM_WINDOWS)
#  include "thread_windows.inl"
#elif defined(FLORAL_PLATFORM_LINUX)
#  include "thread_linux.inl"
#else
// TODO
#endif
