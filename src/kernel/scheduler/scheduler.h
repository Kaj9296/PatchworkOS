#pragma once

#include "process/process.h"

extern void scheduler_yield_to_user_space(void* stackTop);

extern void scheduler_idle_loop();

void scheduler_init();

void scheduler_tick(InterruptFrame* interruptFrame);

void scheduler_exit();

void scheduler_acquire();

void scheduler_release();

void scheduler_wait(uint64_t type, uint64_t data);

void scheduler_yield(InterruptFrame* interruptFrame);

void scheduler_append(Process* process);

void scheduler_remove(Process* process);

void scheduler_schedule();

Process* scheduler_running_process();

void scheduler_switch(Process* process);