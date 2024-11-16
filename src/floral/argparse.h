#pragma once

#include "stdaliases.h"
#include "container.h"
#include "string_utils.h"

///////////////////////////////////////////////////////////////////////////////

struct arena_t;

///////////////////////////////////////////////////////////////////////////////

struct argument_t
{
    str8 name;
    str8 value;
};

struct argument_set_t
{
    dll_t<argument_t> args;
    size argCount;
};

argument_set_t argset_parse(c8** const i_args, const s32 i_numArgs, arena_t* const i_arena);
void debug_argset_dump(argument_set_t* const i_argSet);

str8 argset_get_str(const str8& i_name, const argument_set_t* const i_argSet, const str8& i_default = str8_literal(""));
bool argset_get_bool(str8 i_name, const argument_set_t* const i_argSet, const bool i_default = false);
s32 argset_get_s32(str8 i_name, const argument_set_t* const i_argSet, const s32 i_default = 0);
f32 argset_get_f32(str8 i_name, const argument_set_t* const i_argSet, const f32 i_default = 0.0f);

struct varlist_t
{
    dll_t<str8> parts;
    size partCount;
};

varlist_t varlist_parse(const_cstr i_str, c8 i_delimiter, arena_t* const i_arena);
