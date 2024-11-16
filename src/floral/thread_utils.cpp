#include "thread_utils.h"

///////////////////////////////////////////////////////////////////////////////

void frame_package_initialize(frame_package_t* i_package)
{
    i_package->dataReady = false;
    i_package->mtx = create_mutex();
    i_package->readiedCv = create_cv();
    i_package->consumedCv = create_cv();
}

void frame_package_submit(frame_package_t* i_package)
{
    lock_guard_t guard(&i_package->mtx);
    i_package->dataReady = true;
    cv_notify_one(&i_package->readiedCv);
}

void frame_package_consume(frame_package_t* i_package)
{
    lock_guard_t guard(&i_package->mtx);
    i_package->dataReady = false;
    cv_notify_one(&i_package->consumedCv);
}

void frame_package_wait_ready(frame_package_t* i_package)
{
    lock_guard_t guard(&i_package->mtx);
    while (!i_package->dataReady)
    {
        cv_wait_for(&i_package->readiedCv, &i_package->mtx);
    }
}

void frame_package_wait_consume(frame_package_t* i_package)
{
    lock_guard_t guard(&i_package->mtx);
    while (i_package->dataReady)
    {
        cv_wait_for(&i_package->consumedCv, &i_package->mtx);
    }
}

thread_syncpoint_t create_syncpoint()
{
	thread_syncpoint_t newSp = {};

	newSp.signaled = false;
	newSp.mtx = create_mutex();
	newSp.cv = create_cv();

	return newSp;
}

void syncpoint_wait(thread_syncpoint_t* i_syncPoint)
{
	lock_guard_t guard(&i_syncPoint->mtx);
	while (!i_syncPoint->signaled)
	{
		cv_wait_for(&i_syncPoint->cv, &i_syncPoint->mtx);
	}
}

void syncpoint_signal(thread_syncpoint_t* i_syncPoint)
{
	lock_guard_t guard(&i_syncPoint->mtx);
	i_syncPoint->signaled = true;
	cv_notify_one(&i_syncPoint->cv);
}

void syncpoint_reset(thread_syncpoint_t* i_syncPoint)
{
	lock_guard_t guard(&i_syncPoint->mtx);
	i_syncPoint->signaled = false;
}
