#include "vmm.h"

#include "utils.h"
#include "heap.h"
#include "lock.h"
#include "pmm.h"
#include "sched.h"
#include "regs.h"

static PageTable* kernelPageTable;

static void vmm_load_memory_map(EfiMemoryMap* memoryMap)
{
    kernelPageTable = page_table_new();

    for (uint64_t i = 0; i < memoryMap->descriptorAmount; i++)
    {
        const EfiMemoryDescriptor* desc = EFI_MEMORY_MAP_GET_DESCRIPTOR(memoryMap, i);

        page_table_map_pages(kernelPageTable, desc->virtualStart, desc->physicalStart, desc->amountOfPages, 
            PAGE_FLAG_WRITE | VMM_KERNEL_PAGE_FLAGS);
    }

    page_table_load(kernelPageTable);
}

static void vmm_deallocate_boot_page_table(EfiMemoryMap* memoryMap)
{
    for (uint64_t i = 0; i < memoryMap->descriptorAmount; i++)
    {
        const EfiMemoryDescriptor* desc = EFI_MEMORY_MAP_GET_DESCRIPTOR(memoryMap, i);

        if (desc->type == EFI_MEMORY_TYPE_PAGE_TABLE)
        {
            pmm_free_pages(desc->physicalStart, desc->amountOfPages);
        }
    }
}

void space_init(Space* space)
{
    space->pageTable = page_table_new();
    lock_init(&space->lock);

    for (uint64_t i = PAGE_ENTRY_AMOUNT / 2; i < PAGE_ENTRY_AMOUNT; i++)
    {
        space->pageTable->entries[i] = kernelPageTable->entries[i];
    }
}

void space_cleanup(Space* space)
{
    for (uint64_t i = PAGE_ENTRY_AMOUNT / 2; i < PAGE_ENTRY_AMOUNT; i++)
    {
        space->pageTable->entries[i] = 0;
    }

    page_table_free(space->pageTable);
}

void space_load(Space* space)
{
    if (space == NULL)
    {
        page_table_load(kernelPageTable);
    }
    else
    {
        page_table_load(space->pageTable);
    }
}

void vmm_init(EfiMemoryMap* memoryMap)
{
    vmm_load_memory_map(memoryMap);
    vmm_deallocate_boot_page_table(memoryMap);
}

void* vmm_kernel_map(void* virtualAddress, void* physicalAddress, uint64_t size, uint16_t flags)
{
    if (virtualAddress == NULL)
    {
        virtualAddress = VMM_LOWER_TO_HIGHER(physicalAddress);
    }

    if (page_table_physical_address(kernelPageTable, virtualAddress) == NULL)
    {
        page_table_map_pages(kernelPageTable, virtualAddress, physicalAddress, SIZE_IN_PAGES(size), 
            flags | VMM_KERNEL_PAGE_FLAGS);
    }

    return virtualAddress;
}

void* vmm_allocate(const void* address, uint64_t size)
{
    if (address == NULL)
    {
        //TODO: Choose address
        return NULLPTR(EFAULT);
    }

    Space* space = &sched_process()->space;
    void* alignedAddress = (void*)ROUND_DOWN((uint64_t)address, PAGE_SIZE);

    for (uint64_t i = 0; i < SIZE_IN_PAGES(size); i++)
    {                    
        LOCK_GUARD(&space->lock);

        void* virtualAddress = (void*)((uint64_t)alignedAddress + i * PAGE_SIZE);
        if (page_table_physical_address(space->pageTable, virtualAddress) == NULL)
        {
            //Page table takes ownership of memory
            page_table_map(space->pageTable, virtualAddress, pmm_allocate(), 
                PAGE_FLAG_WRITE | PAGE_FLAG_USER_SUPERVISOR);
        }
    }

    return alignedAddress;
}

void* vmm_physical_to_virtual(const void* address)
{
    Space* space = &sched_process()->space;
    LOCK_GUARD(&space->lock);

    return page_table_physical_address(space->pageTable, address);
}