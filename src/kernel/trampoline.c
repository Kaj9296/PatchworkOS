#include "trampoline.h"

#include <string.h>

#include "utils.h"
#include "heap.h"
#include "vmm.h"

static void* backupBuffer;
static Space space;
//What a mess...

void trampoline_setup(void)
{
    backupBuffer = kmalloc(PAGE_SIZE);
    memcpy(backupBuffer, VMM_LOWER_TO_HIGHER(TRAMPOLINE_PHYSICAL_START), PAGE_SIZE);

    memcpy(VMM_LOWER_TO_HIGHER(TRAMPOLINE_PHYSICAL_START), trampoline_virtual_start, PAGE_SIZE);

    space_init(&space);
    page_table_map(space.pageTable, TRAMPOLINE_PHYSICAL_START, TRAMPOLINE_PHYSICAL_START, PAGE_FLAG_WRITE);
    WRITE_64(VMM_LOWER_TO_HIGHER(TRAMPOLINE_PAGE_TABLE_ADDRESS), VMM_HIGHER_TO_LOWER(space.pageTable));

    WRITE_64(VMM_LOWER_TO_HIGHER(TRAMPOLINE_ENTRY_ADDRESS), smp_entry);
}

void trampoline_cpu_setup(Cpu* cpu)
{
    WRITE_64(VMM_LOWER_TO_HIGHER(TRAMPOLINE_STACK_TOP_ADDRESS), (uint64_t)cpu->idleStack + CPU_IDLE_STACK_SIZE);
}

void trampoline_cleanup(void)
{   
    memcpy(VMM_LOWER_TO_HIGHER(TRAMPOLINE_PHYSICAL_START), backupBuffer, PAGE_SIZE);
    kfree(backupBuffer);

    page_table_unmap(space.pageTable, TRAMPOLINE_PHYSICAL_START);
    space_cleanup(&space);
}