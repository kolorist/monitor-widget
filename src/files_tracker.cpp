#include "files_tracker.h"

#include <floral/log.h>
#include <floral/thread_context.h>

#include <Windows.h>

static FTContext s_fileTrackerContext;

///////////////////////////////////////////////////////////////////////////////

static void FTThreadFunc(voidptr i_data)
{
    FTContext* const ctx = (FTContext*)i_data;

    linear_allocator_t masterAllocator = create_linear_allocator("'tracking' master allocator", SIZE_MB(2));

    thread_context_t threadContext = {
        .allocator = create_linear_allocator(&masterAllocator, "files tracker thread context allocator", SIZE_MB(1))
    };
    thread_set_context(&threadContext);

    log_context_t logCtx = create_log_context("tracking", log_level_e::verbose, &masterAllocator);
    log_set_context(&logCtx);

    windows_logger_t windowsLogger = create_windows_logger(log_level_e::verbose);
    log_context_add_logger(&logCtx, &windows_logger_log_message_cstr, &windows_logger_log_message_wcstr, &windowsLogger);

    LOG_SCOPE(tracking);
    LOG_DEBUG(LITERAL("Tracking thread started for: %s"), ctx->path.data);
    HANDLE hChanges = FindFirstChangeNotification(ctx->path.data, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
    HANDLE events[2] = {
        hChanges,
        ctx->terminateEvent
    };
    FLORAL_ASSERT(hChanges != INVALID_HANDLE_VALUE);

    bool terminate = false;
    while (!terminate)
    {
        DWORD obj = WaitForMultipleObjects(2, events, FALSE, INFINITE);
        switch (obj)
        {
        case WAIT_OBJECT_0 + 0:
        {
            LOG_DEBUG("Changes detected");

            lock_guard_t guard(&ctx->mtx);
            ctx->hasChanges = true;
            if (FindNextChangeNotification(hChanges) == FALSE)
            {
                terminate = true;
            }
            break;
        }

        case WAIT_OBJECT_0 + 1:
        {
            LOG_DEBUG("Terminate signal detected");
            terminate = true;
            break;
        }

        default:
            break;
        }
    }
    LOG_DEBUG("Tracking thread ended.");
}

void FTInitialize(linear_allocator_t* i_allocator)
{
    LOG_SCOPE(files_tracker);
    s_fileTrackerContext.mtx = create_mutex();
    s_fileTrackerContext.hasChanges = true; // first time, we always have changes
    s_fileTrackerContext.path = k_tstrEmpty;

    thread_desc_t threadDesc = {
        .data = &s_fileTrackerContext,
        .func = FTThreadFunc
    };
    s_fileTrackerContext.terminateEvent = CreateEvent(NULL, FALSE, FALSE, LITERAL("terminateEvent"));
    s_fileTrackerContext.thread = create_thread(&threadDesc);

    s_fileTrackerContext.arena = create_arena(i_allocator, SIZE_KB(128));
    s_fileTrackerContext.ready = true;
    LOG_DEBUG("Files Tracker initialized");
}

bool FTHasChanges()
{
    lock_guard_t guard(&s_fileTrackerContext.mtx);
    return s_fileTrackerContext.hasChanges;
}

void FTLock()
{
    mutex_lock(&s_fileTrackerContext.mtx);
    FLORAL_ASSERT_MSG(s_fileTrackerContext.hasChanges, "Trying to lock Files Tracker while having no changes");
}

void FTUnlock()
{
    s_fileTrackerContext.hasChanges = false;
    mutex_unlock(&s_fileTrackerContext.mtx);
}

void FTCleanUp()
{
    LOG_SCOPE(files_tracker);
    LOG_DEBUG("Joining files tracking thread...");
    thread_join(&s_fileTrackerContext.thread);
    LOG_DEBUG("Files tracking thread joined.");
    s_fileTrackerContext.ready = false;
    LOG_DEBUG("Files Tracker stopped");
}

void FTStart(const tstr& i_path)
{
    s_fileTrackerContext.path = tstr_duplicate(&s_fileTrackerContext.arena, i_path);
    thread_start(&s_fileTrackerContext.thread);
}

void FTStop()
{
    SetEvent(s_fileTrackerContext.terminateEvent);
}
