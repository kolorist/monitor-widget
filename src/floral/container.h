#pragma once

#include "assert.h"
#include "stdaliases.h"
#include "thread.h"
#include "memory.h"

///////////////////////////////////////////////////////////////////////////////

template <typename t_value>
struct array_t
{
    t_value* data;
    ssize size;
    ssize capacity;

    t_value& operator[](const ssize i_idx)
    {
        FLORAL_ASSERT_MSG(i_idx >= 0 && i_idx < size, "array_t access out of bound");
        return data[i_idx];
    }

    const t_value& operator[](const ssize i_idx) const
    {
        FLORAL_ASSERT_MSG(i_idx >= 0 && i_idx < size, "array_t access out of bound");
        return data[i_idx];
    }
};

template <typename t_value>
array_t<t_value> create_array(ssize i_capacity, voidptr i_buffer)
{
    array_t<t_value> arr;
    array_initialize(&arr, i_capacity, i_buffer);
    return arr;
}

template <typename t_value>
array_t<t_value> arena_push_array(arena_t* const i_arena, ssize i_capacity)
{
    array_t<t_value> arr;
    voidptr buffer = arena_push(i_arena, i_capacity * sizeof(t_value));
    array_initialize(&arr, i_capacity, buffer);
    return arr;
}

template <typename t_value>
void array_initialize(array_t<t_value>* const io_arr, const ssize i_capacity, voidptr i_buffer)
{
    io_arr->size = 0;
    io_arr->capacity = i_capacity;
    io_arr->data = (t_value*)i_buffer;
}

template <typename t_value>
ssize array_push_back(array_t<t_value>* const i_arr, const t_value& i_value)
{
    FLORAL_ASSERT_MSG(i_arr->size < i_arr->capacity, "array_t overflow");
    const ssize idx = i_arr->size;
    i_arr->data[idx] = i_value;
    i_arr->size++;
    return idx;
}

template <typename t_value>
void array_empty(array_t<t_value>* const i_arr)
{
    i_arr->size = 0;
}

#define arena_create_array(arena, type, capacity) create_array<type>((capacity), arena_push((arena), sizeof(type) * (capacity)))

///////////////////////////////////////////////////////////////////////////////

template <typename t_value, ssize i_capacity>
struct inplace_array_t
{
    ssize size;
    ssize capacity;
    t_value data[i_capacity];

    t_value& operator[](const ssize i_idx)
    {
        FLORAL_ASSERT_MSG(i_idx >= 0 && i_idx < size, "inplace_array_t access out of bound");
        return data[i_idx];
    }

    const t_value& operator[](const ssize i_idx) const
    {
        FLORAL_ASSERT_MSG(i_idx >= 0 && i_idx < size, "inplace_array_t access out of bound");
        return data[i_idx];
    }
};

template <typename t_value, ssize i_capacity>
inplace_array_t<t_value, i_capacity> create_inplace_array()
{
    inplace_array_t<t_value, i_capacity> arr;
    array_initialize(&arr);
    return arr;
}

template <typename t_value, ssize i_capacity>
void array_initialize(inplace_array_t<t_value, i_capacity>* const io_arr)
{
    io_arr->size = 0;
    io_arr->capacity = i_capacity;
}

template <typename t_value, ssize i_capacity>
ssize array_push_back(inplace_array_t<t_value, i_capacity>* const i_arr, const t_value& i_value)
{
    FLORAL_ASSERT_MSG(i_arr->size < i_arr->capacity, "inplace_array_t overflow");
    const ssize idx = i_arr->size;
    i_arr->data[idx] = i_value;
    i_arr->size++;
    return idx;
}

template <typename t_value, ssize i_capacity>
void array_empty(inplace_array_t<t_value, i_capacity>* const i_arr)
{
    i_arr->size = 0;
}

///////////////////////////////////////////////////////////////////////////////

struct cmdbuff_t
{
    p8 data;
    p8 writePtr;
    p8 readPtr;
    size size;
};

cmdbuff_t create_cmdbuff(voidptr i_memory, const size i_size);
void cmdbuff_reset(cmdbuff_t* const io_cmdBuff);
void cmdbuff_copy(cmdbuff_t* const o_to, const cmdbuff_t* const i_from);
bool cmdbuff_read(cmdbuff_t* const i_cmdBuff, voidptr o_buffer, const size i_size);
void cmdbuff_write(cmdbuff_t* const io_cmdBuff, const_voidptr i_buffer, const size i_size);
p8 cmdbuff_reserve(cmdbuff_t* const io_cmdBuff, const size i_size);

template <typename t_value>
bool cmdbuff_read(cmdbuff_t* const i_cmdBuff, t_value* o_value)
{
    return cmdbuff_read(i_cmdBuff, o_value, sizeof(t_value));
}

template <typename t_value>
void cmdbuff_write(cmdbuff_t* const io_cmdBuff, const t_value& i_value)
{
    cmdbuff_write(io_cmdBuff, (const_voidptr)&i_value, sizeof(t_value));
}

template <typename t_value>
t_value* cmdbuff_interpret(cmdbuff_t* const i_cmdBuff)
{
    if (i_cmdBuff->readPtr == i_cmdBuff->writePtr)
    {
        return nullptr;
    }

    p8 rpos = i_cmdBuff->readPtr;
    i_cmdBuff->readPtr = rpos + sizeof(t_value);

    FLORAL_ASSERT((aptr)i_cmdBuff->readPtr <= (aptr)i_cmdBuff->writePtr);
    return (t_value*)rpos;
}

template <typename t_value>
t_value* cmdbuff_interpret(cmdbuff_t* const i_cmdBuff, const size i_size)
{
    if (i_cmdBuff->readPtr == i_cmdBuff->writePtr)
    {
        return nullptr;
    }

    p8 rpos = i_cmdBuff->readPtr;
    i_cmdBuff->readPtr = rpos + i_size;

    FLORAL_ASSERT((aptr)i_cmdBuff->readPtr <= (aptr)i_cmdBuff->writePtr);
    return (t_value*)rpos;
}

template <typename t_value>
t_value* cmdbuff_reserve(cmdbuff_t* const io_cmdBuff)
{
    return (t_value*)cmdbuff_reserve(io_cmdBuff, sizeof(t_value));
}

#define arena_create_cmdbuff(arena, bytes) create_cmdbuff(arena_push((arena), bytes), bytes)

///////////////////////////////////////////////////////////////////////////////

template <typename t_item>
struct circular_queue_mt_t
{
    t_item* data;
    size head;
    size tail;
    size capacity;

    mutex_t mtx;
    condition_variable_t cv;
};

template <typename t_item>
circular_queue_mt_t<t_item> create_circular_queue_mt(const size i_capacity, voidptr i_memory)
{
    circular_queue_mt_t<t_item> queue;
    circular_queue_initialize(&queue, i_memory, i_capacity);
    return queue;
}

template <typename t_item>
void circular_queue_initialize(circular_queue_mt_t<t_item>* const io_queue, voidptr i_memory, const size i_capacity)
{
    io_queue->data = (t_item*)i_memory;
    io_queue->head = 0;
    io_queue->tail = 0;
    io_queue->capacity = i_capacity;

    io_queue->mtx = create_mutex();
    io_queue->cv = create_cv();
}

template <typename t_item>
void circular_queue_enqueue(circular_queue_mt_t<t_item>* const i_queue, const t_item& i_item)
{
    lock_guard_t guard(&i_queue->mtx);
    FLORAL_ASSERT_MSG((i_queue->tail + 1) % i_queue->capacity != (i_queue->head % i_queue->capacity), "circular_queue_t overflow");
    i_queue->data[i_queue->tail % i_queue->capacity] = i_item;
    i_queue->tail++;
    cv_notify_one(&i_queue->cv);
}

template <typename t_item>
t_item circular_queue_dequeue(circular_queue_mt_t<t_item>* const i_queue)
{
    t_item item;
    circular_queue_dequeue_into(i_queue, &item);
    return item;
}

template <typename t_item>
void circular_queue_dequeue_into(circular_queue_mt_t<t_item>* const i_queue, t_item* o_item)
{
    lock_guard_t guard(&i_queue->mtx);
    while (i_queue->head == i_queue->tail)
    {
        cv_wait_for(&i_queue->cv, &i_queue->mtx);
    }
    *o_item = i_queue->data[i_queue->head % i_queue->capacity];
    i_queue->head++;
}

template <typename t_item>
bool circular_queue_try_dequeue_into(circular_queue_mt_t<t_item>* const i_queue, t_item* o_item)
{
    lock_guard_t guard(&i_queue->mtx);
    while (i_queue->head == i_queue->tail)
    {
        return false;
    }
    *o_item = i_queue->data[i_queue->head % i_queue->capacity];
    i_queue->head++;
    return true;
}

#define arena_create_circular_queue_mt(arena, type, capacity) create_circular_queue_mt<type>((capacity), arena_push((arena), sizeof(type) * (capacity)))

///////////////////////////////////////////////////////////////////////////////

template <typename t_handle_type>
struct handle_pool_t
{
    t_handle_type* dense;
    t_handle_type* sparse;

    size capacity;
    size count;
};

template <typename t_handle_type>
handle_pool_t<t_handle_type> create_handle_pool(voidptr i_memory, const size i_capacity)
{
    handle_pool_t<t_handle_type> pool;
    handle_pool_initialize(&pool, i_memory, i_capacity);
    return pool;
}

template <typename t_handle_type>
void handle_pool_reset(handle_pool_t<t_handle_type>* const i_pool)
{
    for (size i = 0; i < i_pool->capacity; i++)
    {
        i_pool->dense[i] = (t_handle_type)i;
    }
    i_pool->count = 0;
}

template <typename t_handle_type>
void handle_pool_initialize(handle_pool_t<t_handle_type>* const o_pool, voidptr i_memory, const size i_capacity)
{
    t_handle_type* denseArr = (t_handle_type*)i_memory;
    for (size i = 0; i < i_capacity; i++)
    {
        denseArr[i] = (t_handle_type)i;
    }

    o_pool->dense = (t_handle_type*)i_memory;
    o_pool->sparse = (t_handle_type*)((aptr)i_memory + i_capacity * sizeof(t_handle_type));
    o_pool->capacity = i_capacity;
    o_pool->count = 0;
}

template <typename t_handle_type>
t_handle_type handle_pool_alloc(handle_pool_t<t_handle_type>* i_handlePool)
{
    if (i_handlePool->count < i_handlePool->capacity)
    {
        size index = i_handlePool->count;
        i_handlePool->count++;

        // new handle value is always located at the end of dense array
        t_handle_type handle = i_handlePool->dense[index];

        // till now, we have dense[index] == handle
        // keep the relationship: sparse[handle] == index and dense[index] == handle
        i_handlePool->sparse[handle] = (t_handle_type)index;

        return handle;
    }
    else
    {
        return t_handle_type(-1);
    }
}

template <typename t_handle_type>
void handle_pool_free(handle_pool_t<t_handle_type>* i_handlePool, const t_handle_type i_handle)
{
    ssize index = i_handlePool->sparse[(size)i_handle];
    ssize lastDenseIndex = i_handlePool->count - 1;

    // move the freed handle value to the last of dense array, it will be reused in the next
    // get_new_handle call also redirect the index of the last allocated handle into the freed
    // handle's slot.
    ssize tmpHandle = i_handlePool->dense[lastDenseIndex];
    i_handlePool->dense[lastDenseIndex] = (size)i_handle;
    i_handlePool->sparse[tmpHandle] = index;
    i_handlePool->dense[index] = tmpHandle; // keep the relationship: sparse[handle] == index and dense[index] == handle

    i_handlePool->count--;
}

template <typename t_handle_type>
bool handle_pool_validate(handle_pool_t<t_handle_type>* i_handlePool, const t_handle_type i_handle)
{
    ssize index = i_handlePool->sparse[(size)i_handle];
    return index < i_handlePool->count && i_handlePool->dense[index] == (size)i_handle;
}

#define arena_create_handle_pool(arena, type, capacity) create_handle_pool<type>(arena_push((arena), (sizeof(type) * (capacity)) << 1), (capacity))

///////////////////////////////////////////////////////////////////////////////

template <typename t_type>
struct dll_t
{
    struct node_t
    {
        node_t* next;
        node_t* prev;
        t_type data;
    };

    node_t* first;
    node_t* last;
};

template <typename t_type>
dll_t<t_type> create_dll()
{
    dll_t<t_type> dll;
    dll.first = nullptr;
    dll.last = nullptr;
    return dll;
}

template <typename t_type>
void dll_reset(dll_t<t_type>* const io_dll)
{
    io_dll->first = nullptr;
    io_dll->last = nullptr;
}

template <typename t_type>
void dll_push_back(dll_t<t_type>* const io_dll, typename dll_t<t_type>::node_t* const i_node)
{
    if (io_dll->last)
    {
        io_dll->last->next = i_node;
        i_node->prev = io_dll->last;
        i_node->next = nullptr;
        io_dll->last = i_node;
    }
    else
    {
        FLORAL_ASSERT(io_dll->first == nullptr);
        io_dll->first = i_node;
        io_dll->last = i_node;
        i_node->next = nullptr;
        i_node->prev = nullptr;
    }
}

template <typename t_type>
void dll_push_front(dll_t<t_type>* const io_dll, typename dll_t<t_type>::node_t* const i_node)
{
    if (io_dll->first)
    {
        io_dll->first->prev = i_node;
        i_node->next = io_dll->first;
        i_node->prev = nullptr;
        io_dll->first = i_node;
    }
    else
    {
        FLORAL_ASSERT(io_dll->last == nullptr);
        io_dll->first = i_node;
        io_dll->last = i_node;
        i_node->next = nullptr;
        i_node->prev = nullptr;
    }
}

template <typename t_node>
void dll_remove(t_node* const i_node)
{
    i_node->prev->next = i_node->next;
    i_node->next->prev = i_node->prev;
    i_node->prev = nullptr;
    i_node->next = nullptr;
}

#define dll_for_each(dll, it) for ((it) = (dll)->first; \
                                   (it);                \
                                   (it) = (it)->next)
#define arena_create_dll_node(arena, type) arena_push_pod(arena, dll_t<type>::node_t)
#define arena_create_array(arena, type, capacity) create_array<type>((capacity), arena_push((arena), sizeof(type) * (capacity)))
