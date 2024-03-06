#pragma once

#include "time/time.h"
#include "lock/lock.h"
#include "queue/queue.h"
#include "process/process.h"
#include "interrupt_frame/interrupt_frame.h"

#define THREAD_STATE_NONE 0
#define THREAD_STATE_RUNNING 1
#define THREAD_STATE_READY 2
#define THREAD_STATE_BLOCKED 3

#define THREAD_PRIORITY_LEVELS 2
#define THREAD_PRIORITY_MIN 0
#define THREAD_PRIORITY_MAX (THREAD_PRIORITY_LEVELS - 1)

#define SCHEDULER_TIME_SLICE (NANOSECONDS_PER_SECOND / 2)

typedef struct
{
    Process* process;
    uint64_t threadCount;
} ThreadCommon;

typedef struct
{
    ThreadCommon* common;

    uint64_t timeStart;
    uint64_t timeEnd;

    void* kernelStackTop;
    void* kernelStackBottom;

    InterruptFrame* interruptFrame;
    Status status;
    uint8_t state;
    uint8_t priority;
} Thread;

typedef struct
{
    Queue* queues[THREAD_PRIORITY_LEVELS];
    Thread* runningThread;
    
    Lock lock;
} Scheduler;

extern void scheduler_idle_loop();

void scheduler_init();

Thread* scheduler_thread();

Process* scheduler_process();

int64_t scheduler_spawn(const char* path);

uint8_t scheduler_wants_to_schedule();

void scheduler_schedule(InterruptFrame* interruptFrame);

//Temporary
uint64_t scheduler_local_thread_amount();