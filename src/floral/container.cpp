#include "container.h"
#include "misc.h"

///////////////////////////////////////////////////////////////////////////////
// multithread command buffer: https://stackoverflow.com/questions/4062126/can-2-pthread-condition-variables-share-the-same-mutex

cmdbuff_t create_cmdbuff(voidptr i_memory, const size i_size)
{
    cmdbuff_t cmdBuff;

    cmdBuff.size = i_size;
    cmdBuff.data = (p8)i_memory;
    cmdBuff.writePtr = (p8)i_memory;
    cmdBuff.readPtr = (p8)i_memory;

    return cmdBuff;
}

void cmdbuff_reset(cmdbuff_t* const io_cmdBuff)
{
    io_cmdBuff->writePtr = io_cmdBuff->data;
    io_cmdBuff->readPtr = io_cmdBuff->data;
}

void cmdbuff_copy(cmdbuff_t* const o_to, const cmdbuff_t* const i_from)
{
    size cmdSize = (aptr)i_from->writePtr - (aptr)i_from->data;
    FLORAL_ASSERT(o_to->size - size(o_to->writePtr - o_to->data) >= cmdSize);
    mem_copy(o_to->writePtr, i_from->data, cmdSize);
    o_to->writePtr += cmdSize;
}

bool cmdbuff_read(cmdbuff_t* const i_cmdBuff, voidptr o_buffer, const size i_size)
{
    if (i_cmdBuff->readPtr == i_cmdBuff->writePtr)
    {
        return false;
    }

    p8 rpos = i_cmdBuff->readPtr;
    mem_copy(o_buffer, rpos, i_size);
    i_cmdBuff->readPtr = rpos + i_size;

    FLORAL_ASSERT((aptr)i_cmdBuff->readPtr <= (aptr)i_cmdBuff->writePtr);
    return true;
}

void cmdbuff_write(cmdbuff_t* const io_cmdBuff, const_voidptr i_buffer, const size i_size)
{
    p8 wpos = io_cmdBuff->writePtr;
    FLORAL_ASSERT((size)wpos + i_size <= (aptr)io_cmdBuff->data + io_cmdBuff->size);
    mem_copy(wpos, i_buffer, i_size);
    io_cmdBuff->writePtr = wpos + i_size;
}

p8 cmdbuff_reserve(cmdbuff_t* const io_cmdBuff, const size i_size)
{
    p8 wpos = io_cmdBuff->writePtr;
    FLORAL_ASSERT((size)wpos + i_size <= (aptr)io_cmdBuff->data + io_cmdBuff->size);
    io_cmdBuff->writePtr = wpos + i_size;
    return wpos;
}
