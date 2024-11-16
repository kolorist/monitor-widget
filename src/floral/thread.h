#pragma once

#include "configs.h"
#include "stdaliases.h"

#if defined(FLORAL_PLATFORM_WINDOWS)
#	include <Windows.h>
#elif defined(FLORAL_PLATFORM_LINUX)
#	include <pthread.h>
#else
// TODO
#endif

///////////////////////////////////////////////////////////////////////////////

struct str8;
struct arena_t;

///////////////////////////////////////////////////////////////////////////////

#if defined(FLORAL_PLATFORM_WINDOWS)
struct thread_platform_data_t
{
	DWORD id;
	HANDLE handle;
};
#elif defined(FLORAL_PLATFORM_LINUX)
struct thread_platform_data_t
{
	pthread_t handle;
};
#else
// TODO
#endif

typedef void (*thread_func_t)(voidptr i_data);

struct thread_desc_t
{
	voidptr data;
	thread_func_t func;
};

struct thread_t
{
	thread_platform_data_t platformData;
	thread_desc_t desc;
};

///////////////////////////////////////////////////////////////////////////////

thread_t create_thread(const thread_desc_t* i_desc);
void initialize_thread(thread_t* const io_thread, const thread_desc_t& i_desc);
void thread_start(thread_t* const io_thread);
void thread_join(thread_t* const io_thread);
void thread_sleep(u32 i_durationMs);
void thread_terminate(s32 i_exitCode);
str8 thread_get_id_as_str(arena_t* const i_arena);

///////////////////////////////////////////////////////////////////////////////

#if defined(FLORAL_PLATFORM_WINDOWS)
struct mutex_platform_data_t
{
	CRITICAL_SECTION handle;
};

struct cv_platform_data_t
{
	HANDLE notifyEvents[2];
	u32 waitersCount;
	CRITICAL_SECTION waitersCountLock;
};
#elif defined(FLORAL_PLATFORM_LINUX)
struct mutex_platform_data_t
{
	pthread_mutex_t handle;
};
struct cv_platform_data_t
{
	pthread_cond_t handle;
};
#else
// TODO
#endif

struct mutex_t
{
	mutex_platform_data_t platformData;
};

struct condition_variable_t
{
	cv_platform_data_t platformData;
};

struct lock_guard_t
{
	lock_guard_t(mutex_t* const i_mtx);
	~lock_guard_t();

	mutex_t* const mtx;
};

///////////////////////////////////////////////////////////////////////////////

mutex_t create_mutex();
void mutex_destroy(mutex_t* const i_mtx);
void mutex_lock(mutex_t* const i_mtx);
void mutex_unlock(mutex_t* const i_mtx);

condition_variable_t create_cv();
void cv_destroy(condition_variable_t* const i_cv);
// will unlock the mutex
void cv_wait_for(condition_variable_t* const i_cv, mutex_t* const i_mtx);
void cv_notify_one(condition_variable_t* const i_cv);
void cv_notify_all(condition_variable_t* const i_cv);
