#include "interrupts.h"

#include <stdint.h>

#include "tty/tty.h"
#include "apic/apic.h"
#include "debug/debug.h"
#include "ipi/ipi.h"
#include "worker_pool/worker_pool.h"
#include "vmm/vmm.h"
#include "worker/syscall/syscall.h"
#include "page_directory/page_directory.h"
#include "worker/scheduler/scheduler.h"
#include "worker/worker.h"
#include "utils/utils.h"

extern void* workerVectorTable[IDT_VECTOR_AMOUNT];

static Idt idt;

void worker_idt_init()
{
    for (uint16_t vector = 0; vector < IDT_VECTOR_AMOUNT; vector++) 
    {        
        idt_set_vector(&idt, (uint8_t)vector, workerVectorTable[vector], IDT_RING0, IDT_INTERRUPT_GATE);
    }        
    
    idt_set_vector(&idt, SYSCALL_VECTOR, workerVectorTable[SYSCALL_VECTOR], IDT_RING3, IDT_INTERRUPT_GATE);
}

Idt* worker_idt()
{
    return &idt;
}

void worker_interrupt_handler(InterruptFrame* interruptFrame)
{            
    if (interruptFrame->vector < IDT_EXCEPTION_AMOUNT)
    {
        worker_exception_handler(interruptFrame);
    }
    else if (interruptFrame->vector == SYSCALL_VECTOR)
    {
        syscall_handler(interruptFrame);
    }
    else if (interruptFrame->vector == IPI_VECTOR)
    {
        worker_ipi_handler(interruptFrame);
    }
}

void worker_ipi_handler(InterruptFrame* interruptFrame)
{
    Ipi ipi = worker_receive_ipi();

    switch (ipi.type)
    {
    case IPI_WORKER_HALT:
    {
        asm volatile("cli");
        while (1)
        {
            asm volatile("hlt");
        }
    }
    break;
    case IPI_WORKER_SCHEDULE:
    {
        Worker* worker = worker_self();

        scheduler_acquire(worker->scheduler);
        scheduler_schedule(worker->scheduler, interruptFrame);
        scheduler_release(worker->scheduler);
    }
    break;
    }        
    
    local_apic_eoi();
}

void worker_exception_handler(InterruptFrame* interruptFrame)
{   
    switch (interruptFrame->errorCode)
    {
    default:
    {
        tty_acquire();
        debug_exception(interruptFrame, "Worker Exception");
        tty_release();

        asm volatile("cli");
        while (1)
        {
            asm volatile("hlt");
        }
    }
    break;
    }
}