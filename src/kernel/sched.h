#pragma once

#include "defs.h"
#include "list.h"
#include "queue.h"
#include "process.h"

#define SCHED_WAIT(condition, timeout) \
({ \
    uint64_t deadline = (timeout) == UINT64_MAX ? UINT64_MAX : (timeout) + time_nanoseconds(); \
    while (!(condition) && deadline > time_nanoseconds()) \
    { \
        sched_yield(); \
    } \
    0; \
})

typedef struct
{
    Queue queues[THREAD_PRIORITY_LEVELS];
    List killedThreads;
    List blockedThreads;
    Thread* runningThread;
} Scheduler;

extern void sched_idle_loop(void);

void scheduler_init(Scheduler* scheduler);

void sched_start(void);

void sched_cpu_start(void);

Thread* sched_thread(void);

Process* sched_process(void);

// Yields the current thread's remaining time slice.
// If no other threads are ready, the CPU will idle until the next call to sched_schedule().
void sched_yield(void);

NORETURN void sched_process_exit(uint64_t status);

NORETURN void sched_thread_exit(void);

uint64_t sched_spawn(const char* path);

uint64_t sched_local_thread_amount(void);

void sched_schedule(TrapFrame* trapFrame);

void sched_push(Thread* thread, uint8_t boost);