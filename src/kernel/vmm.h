#pragma once

#include <common/boot_info.h>

#include "defs.h"
#include "lock.h"
#include "page_table.h"

#define VMM_HIGHER_HALF_BASE 0xFFFF800000000000
#define VMM_LOWER_HALF_MAX 0x800000000000

#define VMM_KERNEL_PAGE_FLAGS (PAGE_FLAG_GLOBAL)

#define VMM_HIGHER_TO_LOWER(address) ((void*)((uint64_t)(address) - VMM_HIGHER_HALF_BASE))
#define VMM_LOWER_TO_HIGHER(address) ((void*)((uint64_t)(address) + VMM_HIGHER_HALF_BASE))

typedef struct
{
    PageTable* pageTable;
    Lock lock;
} Space;

void space_init(Space* space);

void space_cleanup(Space* space);

void space_load(Space* space);

void vmm_init(EfiMemoryMap* memoryMap);

void* vmm_kernel_map(void* virtualAddress, void* physicalAddress, uint64_t size, uint16_t flags);

void* vmm_allocate(const void* address, uint64_t size);

void* vmm_physical_to_virtual(const void* address);