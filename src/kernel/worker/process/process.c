#include "process.h"

#include <stdatomic.h>
#include <libc/string.h>

#include "heap/heap.h"
#include "pmm/pmm.h"
#include "vmm/vmm.h"
#include "gdt/gdt.h"
#include "debug/debug.h"

#include "program_loader/program_loader.h"

atomic_size_t pid;

void pid_init()
{
    atomic_init(&pid, 1);
}

uint64_t pid_new()
{
    return atomic_fetch_add_explicit(&pid, 1, memory_order_seq_cst);
}

Process* process_new(const char* path, uint8_t priority)
{
    if (priority > PROCESS_PRIORITY_MAX)
    {
        debug_panic("Priority level out of bounds");
    }

    Process* process = kmalloc(sizeof(Process));
    memset(process, 0, sizeof(Process));

    process->id = pid_new();

    process->pageDirectory = page_directory_new();
    vmm_map_kernel(process->pageDirectory);
    
    process->memoryBlocks = vector_new(sizeof(MemoryBlock));
    process->fileTable = file_table_new();
    process->interruptFrame = 
        interrupt_frame_new(program_loader_entry, (void*)(VMM_LOWER_HALF_MAX - 1), GDT_USER_CODE | 3, GDT_USER_DATA | 3, process->pageDirectory);
    process->status = STATUS_SUCCESS;
    process->state = PROCESS_STATE_READY;
    process->priority = priority;

    //Temporary: For now the executable is passed via the stack to the program loader.
    //Eventually it will be passed via a system similar to "/proc/self/exec".
    void* stackBottom = process_allocate_pages(process, (void*)(VMM_LOWER_HALF_MAX - PAGE_SIZE), 1);
    void* stackTop = (void*)((uint64_t)stackBottom + 0xFFF);

    uint64_t length = strlen(path);
    void* dest = (void*)((uint64_t)stackTop - length - 1);
    memcpy(dest, path, length + 1);

    process->interruptFrame->stackPointer -= length + 1;
    process->interruptFrame->rdi = VMM_LOWER_HALF_MAX - 1 - length - 1;

    return process;
}

void* process_allocate_pages(Process* process, void* virtualAddress, uint64_t amount)
{
    void* physicalAddress = pmm_allocate_amount(amount);

    MemoryBlock newBlock;
    newBlock.physicalAddress = physicalAddress;
    newBlock.virtualAddress = virtualAddress;
    newBlock.pageAmount = amount;

    vector_push_back(process->memoryBlocks, &newBlock);

    page_directory_map_pages(process->pageDirectory, virtualAddress, physicalAddress, amount, PAGE_FLAG_WRITE | PAGE_FLAG_USER_SUPERVISOR);

    return vmm_physical_to_virtual(physicalAddress);
}

void process_free(Process* process)
{
    page_directory_free(process->pageDirectory);
    for (uint64_t i = 0; i < process->memoryBlocks->length; i++)
    {
        MemoryBlock* memoryBlock = vector_get(process->memoryBlocks, i);

        pmm_free_pages(memoryBlock->physicalAddress, memoryBlock->pageAmount);
    }
    vector_free(process->memoryBlocks);

    file_table_free(process->fileTable);
    interrupt_frame_free(process->interruptFrame);

    kfree(process);
}