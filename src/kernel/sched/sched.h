#pragma once

#include "time/time.h"
#include "lock/lock.h"
#include "queue/queue.h"
#include "array/array.h"
#include "defs/defs.h"
#include "process/process.h"
#include "trap_frame/trap_frame.h"

#define SCHED_TIME_SLICE (NANOSECONDS_PER_SECOND / 2)
#define SCHED_TIMER_HZ 1024

typedef struct
{
    Queue* queues[THREAD_PRIORITY_LEVELS];
    Queue* killedThreads;
    Array* blockedThreads;
    Thread* runningThread;
} Scheduler;

extern void sched_idle_loop(void);

void scheduler_init(Scheduler* scheduler);

void sched_start(void);

void sched_cpu_start(void);

Thread* sched_thread(void);

Process* sched_process(void);

void sched_yield(void);

void sched_sleep(uint64_t nanoseconds);

void sched_block(Blocker blocker);

NORETURN void sched_process_exit(uint64_t status);

NORETURN void sched_thread_exit(void);

uint64_t sched_spawn(const char* path);

uint64_t sched_local_thread_amount(void);