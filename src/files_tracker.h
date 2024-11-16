#pragma once

#include <floral/stdaliases.h>
#include <floral/string_utils.h>
#include <floral/thread.h>

struct FTContext
{
    mutex_t mtx;
    bool hasChanges;

    HANDLE terminateEvent;
    thread_t thread;

    tstr path;

    bool ready;
    arena_t arena;
};

void FTInitialize(linear_allocator_t* i_allocator);
bool FTHasChanges();
void FTLock();
void FTUnlock();
void FTCleanUp();
void FTStart(const tstr& i_path);
void FTStop();
