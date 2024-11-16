#include "atomic.h"

#if defined(FLORAL_PLATFORM_WINDOWS)
#  include "atomic_windows.inl"
#elif defined(FLORAL_PLATFORM_LINUX)
// TODO
#else
// TODO
#endif
