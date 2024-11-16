#pragma once

#include "stdaliases.h"
#include "thread.h"

///////////////////////////////////////////////////////////////////////////////

struct frame_package_t
{
	bool dataReady;
	mutex_t mtx;
	condition_variable_t readiedCv;
	condition_variable_t consumedCv;
};

void frame_package_initialize(frame_package_t* i_package);
void frame_package_submit(frame_package_t* i_package);
void frame_package_consume(frame_package_t* i_package);
void frame_package_wait_ready(frame_package_t* i_package);
void frame_package_wait_consume(frame_package_t* i_package);

struct thread_syncpoint_t
{
	bool signaled;
	mutex_t mtx;
	condition_variable_t cv;
};

thread_syncpoint_t create_syncpoint();
void syncpoint_wait(thread_syncpoint_t* i_syncPoint);
void syncpoint_signal(thread_syncpoint_t* i_syncPoint);
void syncpoint_reset(thread_syncpoint_t* i_syncPoint);
