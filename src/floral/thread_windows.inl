#include "thread.h"
#include "string_utils.h"

// ----------------------------------------------------------------------------

static DWORD WINAPI thread_func(LPVOID i_param)
{
    thread_desc_t* desc = (thread_desc_t*)i_param;
    (*(desc->func))(desc->data);
    return 0;
}

// ----------------------------------------------------------------------------

void thread_sleep(u32 i_durationMs)
{
    Sleep((DWORD)i_durationMs);
}

void thread_terminate(s32 i_exitCode)
{
    ExitThread((DWORD)i_exitCode);
}

str8 thread_get_id_as_str(arena_t* const i_arena)
{
    DWORD id = GetThreadId(GetCurrentThread());
    str8 idStr = str8_printf(i_arena, "thread:%d", id);
    return idStr;
}

// ----------------------------------------------------------------------------

thread_t create_thread(const thread_desc_t* i_desc)
{
    thread_t newThread;
    initialize_thread(&newThread, *i_desc);
    return newThread;
}

void initialize_thread(thread_t* const io_thread, const thread_desc_t& i_desc)
{
    io_thread->platformData = {};
    io_thread->desc = i_desc;
}

void thread_start(thread_t* const io_thread)
{
    DWORD id = 0;
    io_thread->platformData.handle = CreateThread(NULL, 0, &thread_func, (LPVOID)&(io_thread->desc), 0, &id);
    io_thread->platformData.id = id;
}

void thread_join(thread_t* const io_thread)
{
    WaitForSingleObject(io_thread->platformData.handle, INFINITE);
    CloseHandle(io_thread->platformData.handle);

    io_thread->platformData.handle = INVALID_HANDLE_VALUE;
    io_thread->platformData.id = 0;
}

// ----------------------------------------------------------------------------

mutex_t create_mutex()
{
    mutex_t newMutex = {};
    InitializeCriticalSection(&newMutex.platformData.handle);
    return newMutex;
}

void mutex_destroy(mutex_t* const i_mtx)
{
    DeleteCriticalSection(&i_mtx->platformData.handle);
}

void mutex_lock(mutex_t* const i_mtx)
{
    EnterCriticalSection(&i_mtx->platformData.handle);
}

void mutex_unlock(mutex_t* const i_mtx)
{
    LeaveCriticalSection(&i_mtx->platformData.handle);
}

#define EVENT_NOTIFY_ONE 0
#define EVENT_NOTIFY_ALL 1

condition_variable_t create_cv()
{
    condition_variable_t newCv = {};
    newCv.platformData.notifyEvents[EVENT_NOTIFY_ONE] = CreateEvent(NULL, FALSE, FALSE, NULL);
    newCv.platformData.notifyEvents[EVENT_NOTIFY_ALL] = CreateEvent(NULL, TRUE, FALSE, NULL);
    InitializeCriticalSection(&newCv.platformData.waitersCountLock);

    return newCv;
}

void cv_destroy(condition_variable_t* const i_cv)
{
    cv_platform_data_t& pCv = i_cv->platformData;

    CloseHandle(pCv.notifyEvents[EVENT_NOTIFY_ONE]);
    CloseHandle(pCv.notifyEvents[EVENT_NOTIFY_ALL]);
    DeleteCriticalSection(&pCv.waitersCountLock);
}

void cv_wait_for(condition_variable_t* const i_cv, mutex_t* const i_mtx)
{
    cv_platform_data_t& pCv = i_cv->platformData;
    EnterCriticalSection(&pCv.waitersCountLock);
    pCv.waitersCount++;
    LeaveCriticalSection(&pCv.waitersCountLock);

    mutex_unlock(i_mtx);
    // wait
    s32 result = (s32)WaitForMultipleObjects(2, pCv.notifyEvents, FALSE, INFINITE);
    EnterCriticalSection(&pCv.waitersCountLock);
    pCv.waitersCount--;
    bool isLastWaiter = (result == (WAIT_OBJECT_0 + EVENT_NOTIFY_ALL)) && (pCv.waitersCount == 0);
    LeaveCriticalSection(&pCv.waitersCountLock);
    if (isLastWaiter)
    {
        ResetEvent(pCv.notifyEvents[EVENT_NOTIFY_ALL]);
    }
    // end waiting
    mutex_lock(i_mtx);
}

void cv_notify_one(condition_variable_t* const i_cv)
{
    cv_platform_data_t& pCv = i_cv->platformData;

    EnterCriticalSection(&pCv.waitersCountLock);
    bool isHaveWaiters = (pCv.waitersCount > 0);
    LeaveCriticalSection(&pCv.waitersCountLock);
    if (isHaveWaiters)
    {
        SetEvent(pCv.notifyEvents[EVENT_NOTIFY_ONE]);
    }
}

void cv_notify_all(condition_variable_t* const i_cv)
{
    cv_platform_data_t& pCv = i_cv->platformData;
    EnterCriticalSection(&pCv.waitersCountLock);
    bool isHaveWaiters = (pCv.waitersCount > 0);
    LeaveCriticalSection(&pCv.waitersCountLock);
    if (isHaveWaiters)
    {
        SetEvent(pCv.notifyEvents[EVENT_NOTIFY_ALL]);
    }
}

lock_guard_t::lock_guard_t(mutex_t* const i_mtx)
    : mtx(i_mtx)
{
    mutex_lock(mtx);
}

lock_guard_t::~lock_guard_t()
{
    mutex_unlock(mtx);
}
