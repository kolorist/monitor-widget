#pragma once

#include "stdaliases.h"

struct timepoint
{
    u16 year;
    u16 month;
    u16 dayOfWeek;
    u16 day;
    u16 hour;
    u16 minute;
    u16 second;
    u16 millisecond;
};

timepoint time_get_system_now();
timepoint time_get_local_now();
f32 time_get_absolute_ms();         // milliseconds
f64 time_get_absolute_highres_ms(); // milliseconds
