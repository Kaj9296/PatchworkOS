#include "syscall.h"

#include "tty/tty.h"
#include "ram_disk/ram_disk.h"
#include "scheduler/scheduler.h"
#include "interrupt_frame/interrupt_frame.h"
#include "string/string.h"
#include "debug/debug.h"
#include "smp/smp.h"

#include "kernel/kernel.h"

#include "../common.h"

#define WAIT_TYPE_SLEEP 0
#define WAIT_TYPE_IRQ 1
#define WAIT_TYPE_FILE 2

void syscall_handler(InterruptFrame* interruptFrame)
{    
    uint64_t out = 0;

    switch(interruptFrame->rax)
    {    
    case SYS_READ:  
    {

    }
    break;
    case SYS_WRITE:
    {

    }
    break;
    /*case SYS_WAIT:
    {
        uint64_t type = SYSCALL_GET_ARG1(interruptFrame);
        uint64_t data = SYSCALL_GET_ARG1(interruptFrame);
            
        scheduler_acquire();

        scheduler_wait(type, data);
        scheduler_yield(interruptFrame);

        scheduler_release();
    }
    break;*/
    case SYS_FORK:
    {        
        scheduler_acquire();

        Process* child = process_new((void*)interruptFrame->instructionPointer);
        interrupt_frame_copy(child->interruptFrame, interruptFrame);
        child->interruptFrame->rax = 0;
        child->interruptFrame->cr3 = (uint64_t)child->pageDirectory;

        Process* parent = scheduler_running_process();

        MemoryBlock* currentBlock = parent->firstMemoryBlock;
        while (1)
        {
            if (currentBlock == 0)
            {
                break;
            }

            void* physicalAddress = process_allocate_pages(child, currentBlock->virtualAddress, currentBlock->pageAmount);

            memcpy(physicalAddress, currentBlock->physicalAddress, currentBlock->pageAmount * 0x1000);

            currentBlock = currentBlock->next;
        }

        scheduler_append(child);

        scheduler_release();

        out = 1234; //TODO: Replace with child pid, when pid is implemented
    }
    break;
    case SYS_EXIT:
    {        
        scheduler_acquire();

        scheduler_exit();
        scheduler_yield(interruptFrame);

        scheduler_release();
    }
    break;
    case SYS_TEST:
    {
        tty_acquire();

        Cpu* cpu = smp_current_cpu();

        const char* string = page_directory_get_physical_address(SYSCALL_GET_PAGE_DIRECTORY(interruptFrame), (void*)SYSCALL_GET_ARG1(interruptFrame));

        tty_set_cursor_pos(0, 16 * 35 + 16 * cpu->id);
        tty_print("CPU "); tty_printx(cpu->id); tty_print(": "); tty_printx((uint64_t)scheduler_running_process()); tty_print(" | "); tty_print(string);

        tty_release();
    }
    break;
    default:
    {
        out = -1;
    }
    break;
    }

    interruptFrame->rax = out;
}
