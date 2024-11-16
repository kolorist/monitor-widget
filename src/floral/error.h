#pragma once
#include "stdaliases.h"

///////////////////////////////////////////////////////////////////////////////

enum class error_code_e : u32
{
    success = 0,

    // file system
    failed_to_retrive_working_directory,
    failed_to_create_directory,
    failed_to_open_file,

    // containers
    array_access_out_of_range,
    array_element_not_found,
};
