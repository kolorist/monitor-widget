#pragma once

#include "container.h"
#include "error.h"
#include "memory.h"
#include "stdaliases.h"
#include "thread.h"

///////////////////////////////////////////////////////////////////////////////

struct job_director_desc_t
{
    size maxInflightJobs;
    bool disableWorkers;

    u32 workersCount;
    size workerMemorySize;
    void (*workerPrologue)(const u32 i_workerIndex);
    void (*workerEpilogue)(const u32 i_workerIndex);
};

struct job_director_t;
struct job_desc_t
{
    error_code_e (*executor)(job_director_t* const i_jd, const u32 i_jobIndex, voidptr i_input, voidptr i_output);
    voidptr input;
    voidptr output;
};

struct job_t
{
    u32 localIndex;
    ssize counterHandle;
    job_desc_t desc;
};

struct worker_t
{
    thread_t thread;
    u32 index;
    job_director_t* director;
};

struct job_director_t
{
    job_director_desc_t desc;
    inplace_array_t<worker_t, 16> workers;
    circular_queue_mt_t<job_t> queue;

    // TODO: guard counter handles pool with a mutex
    mutex_t chpMtx;
    handle_pool_t<ssize> counterHandlesPool;
    array_t<ATOMIC_TYPE(u32)> countersPool;

    // main worker allocator
    linear_allocator_t allocator;
};

struct job_ops_t
{
    ssize counterHandle;
};

size calculate_memory_size_for_job_director(const job_director_desc_t& i_desc);
void initialize_job_director(job_director_t* const io_jd, const job_director_desc_t& i_desc,
                             voidptr i_memory, const size i_memorySize);
void destroy_job_director(job_director_t* const io_jd);
job_ops_t queue_job(job_director_t* const i_jd, const job_desc_t& i_jobDesc, const u32 i_count = 1);
error_code_e dispatch_job(job_director_t* const i_jd, const job_desc_t& i_jobDesc);
void wait_job(job_director_t* const i_jd, const job_ops_t& i_ops);
