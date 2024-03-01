#include "vmm.h"

#include <stdint.h>
#include <common/boot_info/boot_info.h>

#include "utils/utils.h"
#include "pmm/pmm.h"

extern uint64_t _kernelEnd;

static PageDirectory* kernelPageDirectory;

static void vmm_load_memory_map(EfiMemoryMap* memoryMap)
{
    for (uint64_t i = 0; i < memoryMap->descriptorAmount; i++)
    {
        const EfiMemoryDescriptor* desc = EFI_MEMORY_MAP_GET_DESCRIPTOR(memoryMap, i);
        
        page_directory_map_pages(kernelPageDirectory, desc->virtualStart, desc->physicalStart, desc->amountOfPages, PAGE_FLAG_WRITE | VMM_KERNEL_PAGE_FLAGS);
	}
    page_directory_populate_range(kernelPageDirectory, PAGE_DIRECTORY_ENTRY_AMOUNT / 2, PAGE_DIRECTORY_ENTRY_AMOUNT, PAGE_FLAG_WRITE | VMM_KERNEL_PAGE_FLAGS);
}

static void vmm_deallocate_boot_page_directory(EfiMemoryMap* memoryMap)
{
    for (uint64_t i = 0; i < memoryMap->descriptorAmount; i++)
    {
        const EfiMemoryDescriptor* desc = EFI_MEMORY_MAP_GET_DESCRIPTOR(memoryMap, i);

		if (desc->type == EFI_MEMORY_TYPE_PAGE_DIRECTORY)
		{
            pmm_free_pages(desc->physicalStart, desc->amountOfPages);
		}
	}
}

void vmm_init(EfiMemoryMap* memoryMap)
{
    kernelPageDirectory = page_directory_new();
    
    vmm_load_memory_map(memoryMap);

    vmm_deallocate_boot_page_directory(memoryMap);

    PAGE_DIRECTORY_LOAD(kernelPageDirectory);
}

PageDirectory* vmm_kernel_directory()
{
    return kernelPageDirectory;
}

void* vmm_physical_to_virtual(void* address)
{
    return (void*)((uint64_t)address + VMM_HIGHER_HALF_BASE);
}

void* vmm_virtual_to_physical(void* address)
{
    return (void*)((uint64_t)address - VMM_HIGHER_HALF_BASE);
}

void* vmm_allocate(uint64_t pageAmount)
{
    return vmm_physical_to_virtual(pmm_allocate_amount(pageAmount));
}

void vmm_free(void* address, uint64_t pageAmount)
{
    pmm_free_pages(vmm_virtual_to_physical(address), pageAmount);
}

void* vmm_map(void* physicalAddress, uint64_t pageAmount, uint16_t flags)
{
    void* virtualAddress = vmm_physical_to_virtual(physicalAddress);
    page_directory_map_pages(kernelPageDirectory, virtualAddress, physicalAddress, pageAmount, flags | VMM_KERNEL_PAGE_FLAGS);

    return virtualAddress;
}

void vmm_change_flags(void* address, uint64_t pageAmount, uint16_t flags)
{
    for (uint64_t i = 0; i < pageAmount; i++)
    {
        page_directory_change_flags(kernelPageDirectory, (void*)((uint64_t)address + i * PAGE_SIZE), flags | VMM_KERNEL_PAGE_FLAGS);
    }
}

void vmm_map_kernel(PageDirectory* pageDirectory)
{
    page_directory_copy_range(pageDirectory, kernelPageDirectory, PAGE_DIRECTORY_ENTRY_AMOUNT / 2, PAGE_DIRECTORY_ENTRY_AMOUNT);
}