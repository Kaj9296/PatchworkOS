#include "time.h"

#include "tty/tty.h"
#include "time/time.h"
#include "heap/heap.h"

#include "master/interrupts/interrupts.h"
#include "master/dispatcher/dispatcher.h"

#include "vfs/vfs.h"

#include <libc/string.h>

void time_job_init()
{
    dispatcher_push(time_job, IRQ_SLOW_TIMER);
}

void time_job()
{        
    //Temporary for testing
    tty_acquire();
    tty_set_row(1);
    tty_print("MASTER | SLOW: "); 
    tty_printx(time_nanoseconds()); 
    tty_print(" USED HEAP: "); 
    tty_printx(heap_reserved_size()); 
    tty_release();

    time_accumulate();

    dispatcher_push(time_job, IRQ_SLOW_TIMER);
}