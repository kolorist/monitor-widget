#include "job.h"

#include "atomic.h"
#include "misc.h"

///////////////////////////////////////////////////////////////////////////////

static bool execute_job(job_director_t* i_jd, job_t* const i_job, ATOMIC_TYPE(u32) * io_counter)
{
    job_desc_t* const desc = &i_job->desc;

    if (!desc->executor)
    {
        return false;
    }

    error_code_e result = (*desc->executor)(i_jd, i_job->localIndex, desc->input, desc->output);
    FLORAL_ASSERT(result == error_code_e::success);
    interlocked_decrement(io_counter);
    return true;
}

static void worker_func(voidptr i_data)
{
    auto* const desc = (worker_t* const)i_data;
    job_director_t* const jd = desc->director;
    auto* const queue = &jd->queue;
    auto* const countersPool = &jd->countersPool;

    if (jd->desc.workerPrologue)
    {
        (*jd->desc.workerPrologue)(desc->index);
    }

    while (true)
    {
        job_t job;
        circular_queue_dequeue_into(queue, &job);
        if (!execute_job(jd, &job, &(*countersPool)[job.counterHandle]))
        {
            break;
        }
    }

    if (jd->desc.workerEpilogue)
    {
        (*jd->desc.workerEpilogue)(desc->index);
    }
}

// ----------------------------------------------------------------------------

size calculate_memory_size_for_job_director(const job_director_desc_t& i_desc)
{
    size requiredSize = i_desc.maxInflightJobs * sizeof(job_t);
    requiredSize += i_desc.maxInflightJobs * sizeof(ssize) * 2;
    requiredSize += i_desc.maxInflightJobs * sizeof(ATOMIC_TYPE(u32));
    return requiredSize;
}

void initialize_job_director(job_director_t* const io_jd, const job_director_desc_t& i_desc,
                             voidptr i_memory, const size i_memorySize)
{
    MARK_UNUSED(i_memorySize);
    io_jd->desc = i_desc;

    auto* const workers = &io_jd->workers;
    array_initialize(workers);
    workers->size = i_desc.workersCount;

    p8 memory = (p8)i_memory;
    auto* const queue = &io_jd->queue;
    circular_queue_initialize<job_t>(queue, memory, i_desc.maxInflightJobs * sizeof(job_t));

    memory += i_desc.maxInflightJobs * sizeof(job_t);
    io_jd->chpMtx = create_mutex();
    io_jd->counterHandlesPool = create_handle_pool<ssize>(memory, i_desc.maxInflightJobs * sizeof(ssize) * 2);

    memory += i_desc.maxInflightJobs * sizeof(ssize) * 2;
    array_initialize(&io_jd->countersPool, (ssize)i_desc.maxInflightJobs, (voidptr)memory);
    io_jd->countersPool.size = io_jd->countersPool.capacity;

    if (!i_desc.disableWorkers)
    {
        for (u32 i = 0; i < i_desc.workersCount; i++)
        {
            worker_t& currentWorker = (*workers)[i];
            currentWorker.index = i;
            currentWorker.director = io_jd;

            thread_desc_t desc = {
                .data = &currentWorker,
                .func = &worker_func
            };

            initialize_thread(&currentWorker.thread, desc);
            thread_start(&currentWorker.thread);
        }
    }
}

void destroy_job_director(job_director_t* const io_jd)
{
    if (!io_jd->desc.disableWorkers)
    {
        auto* const workers = &io_jd->workers;
        job_desc_t desc = {};
        queue_job(io_jd, desc, (u32)workers->size);
        for (ssize i = 0; i < workers->size; i++)
        {
            worker_t& currentWorker = (*workers)[i];
            thread_join(&currentWorker.thread);
        }
    }
}

job_ops_t queue_job(job_director_t* const i_jd, const job_desc_t& i_jobDesc, const u32 i_count /* = 1 */)
{
    job_ops_t ops;
    auto* const queue = &i_jd->queue;
    auto* const counterHandlesPool = &i_jd->counterHandlesPool;
    auto* const countersPool = &i_jd->countersPool;

    {
        lock_guard_t guard(&i_jd->chpMtx);
        ops.counterHandle = handle_pool_alloc(counterHandlesPool);
    }
    interlocked_exchange(&(*countersPool)[ops.counterHandle], i_count);

    for (u32 i = 0; i < i_count; i++)
    {
        job_t job = {
            .localIndex = i,
            .counterHandle = ops.counterHandle,
            .desc = i_jobDesc
        };
        circular_queue_enqueue(queue, job);
    }

    return ops;
}

error_code_e dispatch_job(job_director_t* const i_jd, const job_desc_t& i_jobDesc)
{
    return (*i_jobDesc.executor)(i_jd, 0, i_jobDesc.input, i_jobDesc.output);
}

void wait_job(job_director_t* const i_jd, const job_ops_t& i_ops)
{
    auto* const countersPool = &i_jd->countersPool;
    auto* const counterHandlesPool = &i_jd->counterHandlesPool;
    auto* const queue = &i_jd->queue;

    ATOMIC_TYPE(u32)* counter = &(*countersPool)[i_ops.counterHandle];
    while (interlocked_compare_exchange(counter, 0, 0))
    {
        job_t job;
        if (circular_queue_try_dequeue_into(queue, &job))
        {
            if (!execute_job(i_jd, &job, &(*countersPool)[job.counterHandle]))
            {
                break;
            }
        }

        thread_sleep(0);
    }

    {
        lock_guard_t guard(&i_jd->chpMtx);
        handle_pool_free(counterHandlesPool, i_ops.counterHandle);
    }
}
