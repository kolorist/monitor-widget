#include "thread.h"

#include <time.h>

namespace floral
{
// ----------------------------------------------------------------------------
static voidptr thread_func(voidptr i_param)
{
	thread_desc_t* desc = (thread_desc_t*)i_param;
	(*(desc->func))(desc->data);
	pthread_exit(nullptr);
}

void sleep_thread(u32 i_durationMs)
{
	timespec ts;
	ts.tv_sec = i_durationMs / 1000;
	ts.tv_nsec = (i_durationMs % 1000) * 1000000;

	nanosleep(&ts, &ts);
}

void terminate_thread(s32 i_exitCode)
{
	pthread_exit(&i_exitCode);
}

// ----------------------------------------------------------------------------

thread_t create_thread(const thread_desc_t* i_desc)
{
	thread_t newThread;
	newThread.desc = *i_desc;

	return newThread;
}

void start_thread(thread_t* const io_thread)
{
	pthread_create(&io_thread->platformData.handle, nullptr, thread_func, (voidptr)&(io_thread->desc));
}

void join_thread(thread_t* const io_thread)
{
	pthread_join(io_thread->platformData.handle, nullptr);
}

// ----------------------------------------------------------------------------

mutex_t create_mutex()
{
	mutex_t newMutex;
	pthread_mutex_init(&newMutex.platformData.handle, nullptr);
	return newMutex;
}

void destroy_mutex(mutex_t* const i_mtx)
{
	pthread_mutex_destroy(&i_mtx->platformData.handle);
}

void lock_mutex(mutex_t* const i_mtx)
{
	pthread_mutex_lock(&i_mtx->platformData.handle);
}

void unlock_mutex(mutex_t* const i_mtx)
{
	pthread_mutex_unlock(&i_mtx->platformData.handle);
}

condition_variable_t create_condition_variable()
{
	condition_variable_t newCv;
	pthread_cond_init(&newCv.platformData.handle, nullptr);
	return newCv;
}

void destroy_condition_variable(condition_variable_t* const i_cv)
{
	pthread_cond_destroy(&i_cv->platformData.handle);
}

void wait_for_condition(condition_variable_t* const i_cv, mutex_t* const i_mtx)
{
	pthread_cond_wait(&i_cv->platformData.handle, &i_mtx->platformData.handle);
}

void notify_one(condition_variable_t* const i_cv)
{
	pthread_cond_signal(&i_cv->platformData.handle);
}

void notify_all(condition_variable_t* const i_cv)
{
	pthread_cond_broadcast(&i_cv->platformData.handle);
}

lock_guard_t::lock_guard_t(mutex_t* const i_mtx)
	: mtx(i_mtx)
{
	lock_mutex(mtx);
}

lock_guard_t::~lock_guard_t()
{
	unlock_mutex(mtx);
}

// ----------------------------------------------------------------------------
}
