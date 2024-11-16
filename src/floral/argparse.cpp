#include "argparse.h"

#include "log.h"
#include "memory.h"

///////////////////////////////////////////////////////////////////////////////

argument_set_t argset_parse(c8** const i_args, const s32 i_numArgs, arena_t* const i_arena)
{
    FLORAL_ASSERT(i_numArgs > 0);
    dll_t<argument_t> args = create_dll<argument_t>();
    size argCount = 0;
    for (s32 i = 1; i < i_numArgs; i++)
    {
        const_cstr thisArg = i_args[i];
        if (cstr_length(thisArg) > 2 && thisArg[0] == '-' && thisArg[1] == '-')
        {
            dll_t<argument_t>::node_t* argNode = arena_push_pod(i_arena, dll_t<argument_t>::node_t);
            argNode->data.name = str8_duplicate(i_arena, &thisArg[2]);
            argNode->data.value = str8_literal("");

            // peek at the next arg
            if (i + 1 < i_numArgs)
            {
                const_cstr nextArg = i_args[i + 1];
                if (cstr_length(nextArg) > 2 && nextArg[0] == '-' && nextArg[1] == '-')
                {
                    // continue
                }
                else
                {
                    argNode->data.value = str8_duplicate(i_arena, nextArg);
                    i++;
                }
            }
            dll_push_back(&args, argNode);
            argCount++;
        }
        else
        {
            LOG_WARNING("Unsupported argument: %s", thisArg);
        }
    }
    return { .args = args, .argCount = argCount };
}

void debug_argset_dump(argument_set_t* const i_argSet)
{
    if (i_argSet->argCount == 0)
    {
        LOG_DEBUG("  <none>");
    }

    dll_t<argument_t>* const args = &i_argSet->args;
    dll_t<argument_t>::node_t* it = nullptr;
    dll_for_each(args, it)
    {
        if (it->data.value.length > 0)
        {
            LOG_INFO("  - %s: %s", it->data.name.data, it->data.value.data);
        }
        else
        {
            LOG_INFO("  - %s: true", it->data.name.data);
        }
    }
}

str8 argset_get_str(const str8& i_name, const argument_set_t* const i_argSet, const str8& i_default /*= str8_literal("")*/)
{
    const dll_t<argument_t>* const args = &i_argSet->args;
    dll_t<argument_t>::node_t* it = nullptr;
    dll_for_each(args, it)
    {
        if (str8_compare(it->data.name, i_name) == 0)
        {
            return it->data.value;
        }
    }
    return i_default;
}

bool argset_get_bool(str8 i_name, const argument_set_t* const i_argSet, const bool i_default /*= false*/)
{
    const dll_t<argument_t>* const args = &i_argSet->args;
    dll_t<argument_t>::node_t* it = nullptr;
    dll_for_each(args, it)
    {
        if (str8_compare(it->data.name, i_name) == 0)
        {
            return true;
        }
    }
    return i_default;
}

s32 argset_get_s32(str8 i_name, const argument_set_t* const i_argSet, const s32 i_default /*= 0*/)
{
    const dll_t<argument_t>* const args = &i_argSet->args;
    dll_t<argument_t>::node_t* it = nullptr;
    dll_for_each(args, it)
    {
        if (str8_compare(it->data.name, i_name) == 0)
        {
            return str8_to_s32(it->data.value);
        }
    }
    return i_default;
}

f32 argset_get_f32(str8 i_name, const argument_set_t* const i_argSet, const f32 i_default /*= 0.0f*/)
{
    const dll_t<argument_t>* const args = &i_argSet->args;
    dll_t<argument_t>::node_t* it = nullptr;
    dll_for_each(args, it)
    {
        if (str8_compare(it->data.name, i_name) == 0)
        {
            return str8_to_f32(it->data.value);
        }
    }
    return i_default;
}

varlist_t varlist_parse(const_cstr i_str, c8 i_delimiter, arena_t* const i_arena)
{
    dll_t<str8> parts = create_dll<str8>();
    size partCount = 0;
    const c8* ch = i_str;
    const c8* beginCh = i_str;
    while (*ch)
    {
        if (*ch == i_delimiter)
        {
            dll_t<str8>::node_t* partNode = arena_push_pod(i_arena, dll_t<str8>::node_t);
            partNode->data = str8_duplicate(i_arena, beginCh, ch);
            dll_push_back(&parts, partNode);
            partCount++;
            beginCh = ch + 1;
        }
        ch++;
    }

    dll_t<str8>::node_t* partNode = arena_push_pod(i_arena, dll_t<str8>::node_t);
    partNode->data = str8_duplicate(i_arena, beginCh, ch);
    dll_push_back(&parts, partNode);
    partCount++;

    return { .parts = parts, .partCount = partCount };
}
