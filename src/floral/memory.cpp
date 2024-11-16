#include "memory.h"

#include "assert.h"
#include "misc.h"

#if defined(FLORAL_PLATFORM_WINDOWS)
#  include <Windows.h>
#elif defined(FLORAL_PLATFORM_LINUX)
#  include <stdlib.h>
#endif

#if defined(ENABLE_ASAN)
#  include <sanitizer/asan_interface.h>
#  define mem_poison_region(addr, size) __asan_poison_memory_region((addr), (size))
#  define mem_unpoison_region(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
#  define mem_poison_region(addr, size)
#  define mem_unpoison_region(addr, size)
#endif

///////////////////////////////////////////////////////////////////////////////

static voidptr internal_malloc(const size i_bytes)
{
    voidptr addr = nullptr;
#if defined(FLORAL_PLATFORM_WINDOWS)
    addr = (voidptr)VirtualAlloc(nullptr, i_bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif defined(FLORAL_PLATFORM_ANDROID)
    addr = ::malloc(i_bytes);
#endif

    FLORAL_ASSERT_MSG(is_aligned(addr, MEMORY_DEFAULT_MALLOC_ALIGNMENT), "Address must be aligned to k_default_malloc_alignment");
    return addr;
}

static void internal_free(voidptr i_data, size i_bytes)
{
#if defined(FLORAL_PLATFORM_WINDOWS)
    MARK_UNUSED(i_bytes);
    VirtualFree((LPVOID)i_data, 0, MEM_RELEASE);
#elif defined(FLORAL_PLATFORM_ANDROID)
    ::free(i_data);
#endif
}

static void reset_block(alloc_header_t* i_block)
{
#if FILL_MEMORY
    p8 fillAddr0 = (p8)i_block->marker;
    size fc0 = (aptr)i_block - (aptr)fillAddr0;
    memset(fillAddr0, 0xfe, fc0);
    p8 fillAddr = (p8)i_block + sizeof(alloc_header_t);
    size fc1 = i_block->frameSize - fc0 - sizeof(alloc_header_t);
    memset(fillAddr, 0xfe, fc1);
#else
    MARK_UNUSED(i_block);
#endif
}

///////////////////////////////////////////////////////////////////////////////

static void initialize_allocator_internal(const_cstr i_name, voidptr i_baseAddress, const size i_bytes, allocator_t* const io_allocator)
{
    FLORAL_ASSERT(is_aligned(i_baseAddress, MEMORY_DEFAULT_ALIGNMENT));
    FLORAL_ASSERT_MSG((i_bytes & (MEMORY_DEFAULT_MALLOC_ALIGNMENT - 1)) == 0, "Allocator size must be multiples of to k_default_malloc_alignment");
    io_allocator->name = i_name;
    io_allocator->baseAddress = i_baseAddress;
    io_allocator->capacity = i_bytes;
}

static void destroy_allocator_internal(allocator_t* const io_allocator)
{
    FLORAL_ASSERT(io_allocator->name != nullptr);
    internal_free(io_allocator->baseAddress, io_allocator->capacity);
}

static void allocator_reset(allocator_t* const io_allocator)
{
    io_allocator->allocCount = 0;
    io_allocator->freeCount = 0;
    io_allocator->usedBytes = 0;
    io_allocator->effectiveBytes = 0;
}

///////////////////////////////////////////////////////////////////////////////

linear_allocator_t create_linear_allocator(const_cstr i_name, const size i_bytes)
{
    linear_allocator_t allocator;
    voidptr baseAddress = internal_malloc(i_bytes);
    initialize_allocator(i_name, baseAddress, i_bytes, &allocator);
    return allocator;
}

linear_allocator_t create_linear_allocator(const_cstr i_name, voidptr i_baseAddress, const size i_bytes)
{
    linear_allocator_t allocator;
    initialize_allocator(i_name, i_baseAddress, i_bytes, &allocator);
    return allocator;
}

linear_allocator_t create_linear_allocator(linear_allocator_t* i_parent, const_cstr i_name, const size i_bytes)
{
    linear_allocator_t allocator;
    voidptr baseAddress = allocator_alloc(i_parent, i_bytes);
    initialize_allocator(i_name, baseAddress, i_bytes, &allocator);
    return allocator;
}

void initialize_allocator(const_cstr i_name, const size i_bytes, linear_allocator_t* const io_allocator)
{
    voidptr baseAddress = internal_malloc(i_bytes);
    initialize_allocator(i_name, baseAddress, i_bytes, io_allocator);
}

void initialize_allocator(const_cstr i_name, voidptr i_baseAddress, const size i_bytes, linear_allocator_t* const io_allocator)
{
    initialize_allocator_internal(i_name, i_baseAddress, i_bytes, &io_allocator->base);
    allocator_reset(io_allocator);
}

void allocator_reset(linear_allocator_t* io_allocator)
{
    allocator_reset(&io_allocator->base);

    io_allocator->marker = (p8)io_allocator->base.baseAddress;
    io_allocator->lastAlloc = nullptr;
}

void allocator_destroy(linear_allocator_t* io_allocator)
{
    destroy_allocator_internal(&io_allocator->base);
}

void allocator_destroy(linear_allocator_t* i_parent, linear_allocator_t* i_child)
{
    allocator_free(i_parent, i_child->base.baseAddress);
}

voidptr allocator_alloc(linear_allocator_t* const i_allocator, const size i_bytes)
{
    return allocator_alloc(i_allocator, i_bytes, MEMORY_DEFAULT_ALIGNMENT);
}

/*!
 * [padding][header][padding][addressOfHeader][data]
 * -------------------frameSize---------------------
 *
 * [header]: is always aligned by `MEMORY_DEFAULT_ALIGNMENT`
 *
 * [padding]: alignment padding
 *
 * [addressOfHeader]: located right before [data], it is for the allocator to know where the header
 *                    is, to perform `free()` operation
 *
 * [data]: the actual data, is always aligned to `i_alignment`
 */
voidptr allocator_alloc(linear_allocator_t* const i_allocator, const size i_bytes, const size i_alignment)
{
    FLORAL_ASSERT(i_bytes > 0 && i_alignment > 0);
    const size alignment = math_max(i_alignment, MEMORY_DEFAULT_ALIGNMENT);
    const size metaDataSize = sizeof(alloc_header_t) + sizeof(voidptr);
    p8 marker = i_allocator->marker;
    p8 headerAddr = (p8)align_addr(marker, MEMORY_DEFAULT_ALIGNMENT);   // align forward, this is address of the header
    p8 dataAddr = (p8)align_addr(headerAddr + metaDataSize, alignment); // align forward, this is the start of the data
    voidptr* ptrToHeader = (voidptr*)(dataAddr - sizeof(voidptr));      // pointer to header is stored right before the data
    const size frameSize = size(dataAddr - marker + i_bytes);
    allocator_t* const base = &i_allocator->base;

    if (base->usedBytes + frameSize > base->capacity)
    {
        FLORAL_ASSERT_MSG(false, "Out of memory!");
        return nullptr;
    }

    // save info about displacement and allocated frame size
    alloc_header_t* header = (alloc_header_t*)headerAddr;
    header->prev = i_allocator->lastAlloc;
    header->next = nullptr;
    header->marker = marker;
    header->frameSize = frameSize;
    header->alignment = alignment;
    header->dataSize = i_bytes;

    *ptrToHeader = (voidptr)header;

    if (i_allocator->lastAlloc != nullptr)
    {
        i_allocator->lastAlloc->next = header;
    }

    base->allocCount++;
    base->usedBytes += frameSize;
    base->effectiveBytes += i_bytes;

    i_allocator->marker += frameSize;
    i_allocator->lastAlloc = header;
    return dataAddr;
}

voidptr allocator_realloc(linear_allocator_t* const i_allocator, voidptr i_data, const size i_newBytes)
{
    return allocator_realloc(i_allocator, i_data, i_newBytes, MEMORY_DEFAULT_ALIGNMENT);
}

voidptr allocator_realloc(linear_allocator_t* const i_allocator, voidptr i_data, const size i_newBytes, const size i_alignment)
{
    FLORAL_ASSERT(i_data != nullptr && i_newBytes > 0 && i_alignment > 0);
    alloc_header_t** const ppHeader = (alloc_header_t**)((p8)i_data - sizeof(voidptr));
    alloc_header_t* const header = *ppHeader;

    // copy data to new position
    voidptr newData = allocator_alloc(i_allocator, i_newBytes, i_alignment);
    const size copySize = math_min(i_newBytes, header->dataSize);
    mem_copy(newData, i_data, copySize);
    return newData;
}

void allocator_free(linear_allocator_t* const i_allocator, voidptr i_data)
{
    if (i_data == nullptr)
    {
        return;
    }

    alloc_header_t** const ppHeader = (alloc_header_t**)((p8)i_data - sizeof(voidptr));
    alloc_header_t* const header = *ppHeader;
    const size frameSize = header->frameSize;
    allocator_t* const base = &i_allocator->base;

    FLORAL_ASSERT_MSG(i_allocator->marker - header->frameSize == header->marker, "Invalid free: not in allocation order");

    // adjust header
    if (header->prev)
    {
        header->prev->next = nullptr;
    }
    base->freeCount++;
    FLORAL_ASSERT(base->usedBytes >= frameSize);
    base->usedBytes -= frameSize;
    base->effectiveBytes -= header->dataSize;

    i_allocator->marker -= frameSize;
    i_allocator->lastAlloc = header->prev;
}

///////////////////////////////////////////////////////////////////////////////

freelist_allocator_t create_freelist_allocator(const_cstr i_name, const size i_bytes)
{
    freelist_allocator_t allocator;
    voidptr baseAddress = internal_malloc(i_bytes);
    initialize_allocator(i_name, baseAddress, i_bytes, &allocator);
    return allocator;
}

freelist_allocator_t create_freelist_allocator(const_cstr i_name, voidptr i_baseAddress, const size i_bytes)
{
    freelist_allocator_t allocator;
    initialize_allocator(i_name, i_baseAddress, i_bytes, &allocator);
    return allocator;
}

freelist_allocator_t create_freelist_allocator(linear_allocator_t* const i_parent, const_cstr i_name, const size i_bytes)
{
    freelist_allocator_t allocator;
    voidptr baseAddress = allocator_alloc(i_parent, i_bytes);
    initialize_allocator(i_name, baseAddress, i_bytes, &allocator);
    return allocator;
}

void initialize_allocator(const_cstr i_name, const size i_bytes, freelist_allocator_t* const io_allocator)
{
    voidptr baseAddress = internal_malloc(i_bytes);
    initialize_allocator(i_name, baseAddress, i_bytes, io_allocator);
}

void initialize_allocator(const_cstr i_name, voidptr i_baseAddress, const size i_bytes, freelist_allocator_t* const io_allocator)
{
    initialize_allocator_internal(i_name, i_baseAddress, i_bytes, &io_allocator->base);
    allocator_reset(io_allocator);
}

void allocator_reset(freelist_allocator_t* const io_allocator)
{
    allocator_t* const base = &io_allocator->base;

    aptr endAddr = (aptr)base->baseAddress + (aptr)base->capacity;
    const size metaDataSize = sizeof(alloc_header_t) + sizeof(voidptr);
    p8 nextHeaderAddr = (p8)align_addr(base->baseAddress, MEMORY_DEFAULT_ALIGNMENT);
    p8 nextDataAddr = (p8)align_addr(nextHeaderAddr + (aptr)metaDataSize, MEMORY_DEFAULT_ALIGNMENT);
    FLORAL_ASSERT_MSG((aptr)nextDataAddr < endAddr, "Allocator size is too small, there is no space for allocation header.");

    alloc_header_t* firstFreeBlock = (alloc_header_t*)nextHeaderAddr;
    firstFreeBlock->marker = (p8)base->baseAddress;
    firstFreeBlock->frameSize = endAddr - (aptr)firstFreeBlock->marker;
    firstFreeBlock->dataSize = 0;
    firstFreeBlock->alignment = 0;
    firstFreeBlock->prev = nullptr;
    firstFreeBlock->next = nullptr;
    reset_block(firstFreeBlock);

    allocator_reset(base);
    io_allocator->firstFreeBlock = firstFreeBlock;
    io_allocator->lastAlloc = nullptr;
}

void allocator_destroy(freelist_allocator_t* const io_allocator)
{
    destroy_allocator_internal(&io_allocator->base);
}

void allocator_destroy(linear_allocator_t* const i_parent, freelist_allocator_t* const i_child)
{
    allocator_free(i_parent, i_child->base.baseAddress);
}

voidptr allocator_alloc(freelist_allocator_t* const i_allocator, const size i_bytes)
{
    return allocator_alloc(i_allocator, i_bytes, MEMORY_DEFAULT_ALIGNMENT);
}

static bool can_fit(alloc_header_t* i_header, const size i_bytes, const size i_alignment)
{
    const size metaDataSize = sizeof(alloc_header_t) + sizeof(voidptr);
    aptr headerAddr = (aptr)i_header;                                         // start address of the frame
    aptr dataAddr = align_aptr(headerAddr + (aptr)metaDataSize, i_alignment); // align forward, this is the start of the data
    size frameSizeNeeded = dataAddr - (aptr)i_header->marker + i_bytes;
    return (frameSizeNeeded <= i_header->frameSize);
}

static bool can_create_new_block(alloc_header_t* i_header, const size i_bytes, const size i_alignment)
{
    const size metaDataSize = sizeof(alloc_header_t) + sizeof(voidptr);
    aptr headerAddr = (aptr)i_header;                                         // start address of the frame
    aptr dataAddr = align_aptr(headerAddr + (aptr)metaDataSize, i_alignment); // align forward, this is the start of the data
    size frameSizeNeeded = dataAddr - (aptr)i_header->marker + i_bytes;
    aptr endFrameAddr = (aptr)(i_header->marker) + (aptr)i_header->frameSize;
    aptr nextHeaderAddr = align_aptr((aptr)i_header->marker + (aptr)frameSizeNeeded, MEMORY_DEFAULT_ALIGNMENT);
    aptr nextDataAddr = align_aptr(nextHeaderAddr + (aptr)metaDataSize, MEMORY_DEFAULT_ALIGNMENT);
    return (nextDataAddr <= endFrameAddr);
}

// first-fit strategy
voidptr allocator_alloc(freelist_allocator_t* const i_allocator, const size i_bytes, const size i_alignment)
{
    FLORAL_ASSERT(i_bytes > 0);

    const size alignment = math_max(i_alignment, MEMORY_DEFAULT_ALIGNMENT);
    alloc_header_t* currBlock = i_allocator->firstFreeBlock;
    // search
    while (currBlock && !can_fit(currBlock, i_bytes, alignment))
    {
        currBlock = currBlock->next;
    }

    if (currBlock) // found it!
    {
        const size metaDataSize = sizeof(alloc_header_t) + sizeof(voidptr);
        p8 dataAddr = (p8)align_addr((p8)currBlock + metaDataSize, alignment);
        voidptr* ptrToHeader = (voidptr*)(dataAddr - sizeof(voidptr));
        allocator_t* const base = &i_allocator->base;

        // C1: a new free block needs to be created
        if (can_create_new_block(currBlock, i_bytes, alignment))
        {
            p8 orgAddr = currBlock->marker;
            size frameSize = (aptr)dataAddr - (aptr)orgAddr + i_bytes;

            size oldFrameSize = currBlock->frameSize;
            currBlock->frameSize = frameSize; // update frame_size of currBlock

            // create new free block
            aptr endFrameAddr = (aptr)(currBlock->marker) + (aptr)oldFrameSize;
            alloc_header_t* newBlock = (alloc_header_t*)align_addr(currBlock->marker + frameSize, MEMORY_DEFAULT_ALIGNMENT);
            // update pointers of new free block
            newBlock->marker = currBlock->marker + frameSize;
            newBlock->frameSize = endFrameAddr - (aptr)newBlock->marker;
            newBlock->dataSize = 0;
            newBlock->alignment = 0;
            newBlock->next = currBlock->next;
            newBlock->prev = currBlock->prev;

            // delete pointers on currBlock as it's already occupied
            currBlock->next = nullptr;
            currBlock->prev = nullptr;

            // update linked list
            if (newBlock->prev)
            {
                newBlock->prev->next = newBlock;
            }
            if (newBlock->next)
            {
                newBlock->next->prev = newBlock;
            }

            // update first free block
            if (currBlock == i_allocator->firstFreeBlock)
            {
                i_allocator->firstFreeBlock = newBlock;
            }
        }
        else
        {
            // C2: we can use all of this block
            if (currBlock->prev)
            {
                currBlock->prev->next = currBlock->next;
            }
            if (currBlock->next)
            {
                currBlock->next->prev = currBlock->prev;
            }
            // update first free block?
            if (currBlock == i_allocator->firstFreeBlock)
            {
                i_allocator->firstFreeBlock = currBlock->next;
            }

            // delete pointers
            currBlock->next = nullptr;
            currBlock->prev = nullptr;
        }

        currBlock->dataSize = i_bytes;
        currBlock->alignment = alignment;
        currBlock->next = nullptr;
        currBlock->prev = i_allocator->lastAlloc;

        *ptrToHeader = (voidptr)currBlock;

#if FILL_MEMORY
        memset(dataAddr, 0, i_bytes);
#endif

        if (i_allocator->lastAlloc != nullptr)
        {
            i_allocator->lastAlloc->next = currBlock;
        }

        base->allocCount++;
        base->usedBytes += currBlock->frameSize;
        base->effectiveBytes += currBlock->dataSize;

        i_allocator->lastAlloc = currBlock;

        return dataAddr;
    }

    FLORAL_ASSERT_MSG(false, "Out of free memory!");
    return nullptr; // nothing found, cannot allocate anything
}

voidptr allocator_realloc(freelist_allocator_t* const i_allocator, voidptr i_data, const size i_newBytes)
{
    return allocator_realloc(i_allocator, i_data, i_newBytes, 0);
}

voidptr allocator_realloc(freelist_allocator_t* const i_allocator, voidptr i_data, const size i_newBytes, const size i_alignment)
{
    FLORAL_ASSERT(i_data != nullptr);
    alloc_header_t** ppHeader = (alloc_header_t**)((p8)i_data - sizeof(voidptr));
    alloc_header_t* header = *ppHeader;

    size alignment = i_alignment;
    if (i_alignment == 0)
    {
        alignment = header->alignment;
    }

    voidptr newData = allocator_alloc(i_allocator, i_newBytes, alignment);
    const size copySize = math_min(i_newBytes, header->dataSize);
    memcpy(newData, i_data, copySize);
    allocator_free(i_allocator, i_data);
    return newData;
}

// free a block, update the free list
static void free_block(alloc_header_t* i_block, alloc_header_t* i_prevFree, alloc_header_t* i_nextFree)
{
    i_block->next = nullptr;
    i_block->prev = nullptr;
    i_block->alignment = 0;
    i_block->dataSize = 0;

    reset_block(i_block);

    // adjust pointers
    if (i_prevFree)
    {
        i_block->prev = i_prevFree;
        i_prevFree->next = i_block;
    }
    if (i_nextFree)
    {
        i_block->next = i_nextFree;
        i_nextFree->prev = i_block;
    }
}

// check if 2 blocks can be joined
static bool can_join(alloc_header_t* i_leftBlock, alloc_header_t* i_rightBlock)
{
    aptr leftEnd = ((aptr)i_leftBlock->marker + (aptr)i_leftBlock->frameSize);
    aptr rightStart = (aptr)i_rightBlock->marker;
    return (leftEnd == rightStart);
}

// join 2 *free* blocks together
static bool join_blocks(alloc_header_t* i_leftBlock, alloc_header_t* i_rightBlock)
{
    if (!i_leftBlock || !i_rightBlock)
    {
        return false;
    }

    if (can_join(i_leftBlock, i_rightBlock))
    {
        i_leftBlock->next = i_rightBlock->next;            // adjust left block's pointers
        i_leftBlock->frameSize += i_rightBlock->frameSize; // update frame_size of left block
        if (i_rightBlock->next)
        {
            i_rightBlock->next->prev = i_leftBlock;
        }
        reset_block(i_leftBlock);
        return true;
    }
    else
    {
        return false;
    }
}

void allocator_free(freelist_allocator_t* const i_allocator, voidptr i_data)
{
    if (i_data == nullptr)
    {
        return;
    }

    alloc_header_t** ppReleaseBlock = (alloc_header_t**)((p8)i_data - sizeof(voidptr));
    alloc_header_t* releaseBlock = *ppReleaseBlock;
    size frameSize = releaseBlock->frameSize;
    size dataSize = releaseBlock->dataSize;
    allocator_t* const base = &i_allocator->base;

    if (releaseBlock->next)
    {
        releaseBlock->next->prev = releaseBlock->prev;
    }
    if (releaseBlock->prev)
    {
        releaseBlock->prev->next = releaseBlock->next;
    }
    if (releaseBlock == i_allocator->lastAlloc)
    {
        i_allocator->lastAlloc = releaseBlock->prev;
    }

    // search for nearest-after free block
    alloc_header_t* nextFree = i_allocator->firstFreeBlock;
    while (nextFree && ((aptr)nextFree <= (aptr)releaseBlock))
    {
        nextFree = nextFree->next;
    }

    alloc_header_t* prevFree = nullptr;
    if (nextFree != nullptr)
    {
        prevFree = nextFree->prev;
    }

    // free releaseBlock
    free_block(releaseBlock, prevFree, nextFree);

    // update first free block
    if ((aptr)releaseBlock < (aptr)i_allocator->firstFreeBlock || i_allocator->firstFreeBlock == nullptr)
    {
        i_allocator->firstFreeBlock = releaseBlock;
    }
    // join blocks if possible
    if (join_blocks(prevFree, releaseBlock))
    {
        join_blocks(prevFree, nextFree);
    }
    else
    {
        join_blocks(releaseBlock, nextFree);
    }

    base->freeCount++;
    FLORAL_ASSERT(base->usedBytes >= frameSize);
    base->usedBytes -= frameSize;
    base->effectiveBytes -= dataSize;
}

// ----------------------------------------------------------------------------
// arena

arena_t create_arena(linear_allocator_t* const i_allocator, const size i_bytes)
{
    p8 ptr = (p8)allocator_alloc(i_allocator, i_bytes);
    mem_poison_region(ptr, i_bytes);
    arena_t arena = { .baseAddress = ptr,
                      .marker = 0,
                      .capacity = i_bytes,
                      .alignment = MEMORY_DEFAULT_ALIGNMENT,
                      .parent = i_allocator };
    return arena;
}

arena_t create_arena(voidptr i_baseAddress, const size i_bytes)
{
    mem_poison_region(i_baseAddress, i_bytes);
    arena_t arena = { .baseAddress = (p8)i_baseAddress,
                      .marker = 0,
                      .capacity = i_bytes,
                      .alignment = MEMORY_DEFAULT_ALIGNMENT,
                      .parent = nullptr };
    return arena;
}

void arena_reset(arena_t* const i_arena)
{
    i_arena->marker = 0;
}

void arena_destroy(arena_t* const i_arena)
{
    if (i_arena->parent)
    {
        allocator_free(i_arena->parent, i_arena->baseAddress);
    }
}

aptr arena_tellp(arena_t* const i_arena)
{
    return i_arena->marker;
}

voidptr arena_push(arena_t* const i_arena, const size i_bytes)
{
    return arena_push(i_arena, i_bytes, MEMORY_DEFAULT_ALIGNMENT);
}

voidptr arena_push(arena_t* const i_arena, const size i_bytes, const size i_alignment)
{
    FLORAL_ASSERT((i_alignment & (MEMORY_DEFAULT_ALIGNMENT - 1)) == 0);

    voidptr addr = &i_arena->baseAddress[i_arena->marker];
    addr = align_addr(addr, i_alignment);
    i_arena->marker = (p8)addr - i_arena->baseAddress + (aptr)i_bytes;
    FLORAL_ASSERT(i_arena->marker <= (aptr)i_arena->capacity);
    mem_unpoison_region(addr, i_bytes);

    FLORAL_ASSERT(is_aligned(addr, i_alignment));
    return addr;
}

void arena_pop_to(arena_t* const i_arena, const aptr i_pos)
{
    FLORAL_ASSERT(i_pos <= i_arena->marker && i_pos >= 0);
    mem_poison_region(&i_arena->baseAddress[i_pos], i_arena->marker - i_pos);
    i_arena->marker = i_pos;
}

void arena_pop(arena_t* const i_arena, const size i_bytes)
{
    aptr pos = i_arena->marker - (aptr)i_bytes;
    arena_pop_to(i_arena, pos);
}

bool arena_contain(arena_t* const i_arena, const_voidptr i_ptr)
{
    aptr diff = (aptr)i_ptr - (aptr)i_arena->baseAddress;
    return (diff >= 0 && diff < i_arena->marker);
}

scratch_region_t scratch_begin(arena_t* const i_arena)
{
    aptr origin = arena_tellp(i_arena);
    scratch_region_t reg;
    reg.arena = i_arena;
    reg.origin = origin;
    return reg;
}

void scratch_end(scratch_region_t* const i_scratch)
{
    arena_pop_to(i_scratch->arena, i_scratch->origin);
}

