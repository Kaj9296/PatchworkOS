#include "pmm.h"

#include <stddef.h>
#include <string.h>

#include <common/boot_info/boot_info.h>

#include "debug/debug.h"
#include "lock/lock.h"
#include "vmm/vmm.h"
#include "utils/utils.h"

static uint64_t* bitmap;
static uint64_t bitmapSize;
static void* firstFreePage;

static uint64_t pageAmount = 0;
static uint64_t usablePageAmount = 0;

static Lock lock;

static inline uint8_t is_type_usable(uint64_t memoryType)
{
	return !(memoryType == EFI_UNUSABLE_MEMORY ||
		memoryType == EFI_ACPI_RECLAIM_MEMORY ||
		memoryType == EFI_ACPI_MEMORY_NVS ||
		memoryType == EFI_MEMORY_MAPPED_IO ||
		memoryType == EFI_MEMORY_MAPPED_IO_PORT_SPACE ||
		memoryType == EFI_PAL_CODE ||
		memoryType == EFI_RESERVED ||
		memoryType == EFI_PERSISTENT_MEMORY);
}

static inline uint8_t is_type_reserved(uint64_t memoryType)
{
	return !(memoryType == EFI_CONVENTIONAL_MEMORY ||
		memoryType == EFI_LOADER_CODE ||
		memoryType == EFI_LOADER_DATA ||
		memoryType == EFI_BOOT_SERVICES_CODE ||
		memoryType == EFI_BOOT_SERVICES_DATA);
}

static inline bool pmm_is_reserved(void* address)
{   
    return (bitmap[QWORD_INDEX(address)] >> BIT_INDEX(address)) & 1ULL;
}

static inline void pmm_reserve_page(void* address)
{
    bitmap[QWORD_INDEX(address)] |= 1ULL << BIT_INDEX(address);
}

static inline void pmm_reserve_pages(void* address, uint64_t count)
{
    for (uint64_t i = 0; i < count; i++)
    {
        pmm_reserve_page((void*)((uint64_t)address + i * PAGE_SIZE));
    }
}

static void pmm_allocate_bitmap(EfiMemoryMap* memoryMap)
{
    uintptr_t highestAddress = 0;
    for (uint64_t i = 0; i < memoryMap->descriptorAmount; i++)
    {
        const EfiMemoryDescriptor* desc = EFI_MEMORY_MAP_GET_DESCRIPTOR(memoryMap, i);
        highestAddress = MAX(highestAddress, (uintptr_t)desc->physicalStart + desc->amountOfPages * PAGE_SIZE);        
        
        if (is_type_usable(desc->type))
        {
            usablePageAmount += desc->amountOfPages;
        }
    }
    pageAmount = highestAddress / PAGE_SIZE;    

    bitmapSize = pageAmount / 8;
    for (uint64_t i = 0; i < memoryMap->descriptorAmount; i++)
    {
        const EfiMemoryDescriptor* desc = EFI_MEMORY_MAP_GET_DESCRIPTOR(memoryMap, i);
        
        if (!is_type_reserved(desc->type) && bitmapSize < desc->amountOfPages * PAGE_SIZE)
        {
            bitmap = desc->virtualStart;    
            memset(bitmap, INT32_MAX, bitmapSize);
            return;
        }
    }
}

static void pmm_load_memory_map(EfiMemoryMap* memoryMap)
{
    for (uint64_t i = 0; i < memoryMap->descriptorAmount; i++)
    {
        const EfiMemoryDescriptor* desc = EFI_MEMORY_MAP_GET_DESCRIPTOR(memoryMap, i);

        if (!is_type_reserved(desc->type))
        {
            pmm_free_pages(desc->physicalStart, desc->amountOfPages);
        }
    }

    pmm_reserve_pages(VMM_HIGHER_TO_LOWER(bitmap), SIZE_IN_PAGES(bitmapSize));
    pmm_reserve_page(0); //Unecessary
}

void pmm_init(EfiMemoryMap* memoryMap)
{   
    lock = lock_create();

    pmm_allocate_bitmap(memoryMap);

    pmm_load_memory_map(memoryMap);
}

void* pmm_allocate(void)
{
    lock_acquire(&lock);

    for (uint64_t qwordIndex = QWORD_INDEX(firstFreePage); qwordIndex < pageAmount / 64; qwordIndex++)
    {
        if (bitmap[qwordIndex] != UINT64_MAX) 
        {
            uint64_t bitIndex = bitmap[qwordIndex] != 0 ? __builtin_ctzll(~bitmap[qwordIndex]) : 0;
            
            void* address = (void*)((qwordIndex * 64 + bitIndex) * PAGE_SIZE);
            pmm_reserve_page(address);

            lock_release(&lock);
            return address;
        }
    }

    lock_release(&lock);
    debug_panic("Physical Memory Manager full!");
}

void* pmm_allocate_amount(uint64_t amount)
{
    if (amount <= 1)
    {
        return pmm_allocate();
    }

    //TODO: Optimize this, remove this?

    lock_acquire(&lock);

    uintptr_t startAddress = UINT64_MAX;
    uint64_t freePagesFound = 0;
    for (uintptr_t address = 0; address < pageAmount * PAGE_SIZE; address += PAGE_SIZE)
    {
        if (pmm_is_reserved((void*)address))
        {
            startAddress = UINT64_MAX;
            freePagesFound = 0;
        }
        else
        {
            if (startAddress == UINT64_MAX)
            {
                startAddress = address;
                freePagesFound = 0;
            }

            freePagesFound++;
            if (freePagesFound == amount)
            {
                pmm_reserve_pages((void*)startAddress, freePagesFound);       

                lock_release(&lock);
                return (void*)startAddress;
            }
        }
    }

    lock_release(&lock);
    debug_panic("Page allocator full!");
}

void pmm_free_page(void* address)
{    
    lock_acquire(&lock);
    bitmap[QWORD_INDEX(address)] &= ~(1ULL << BIT_INDEX(address));
    lock_release(&lock);
}

void pmm_free_pages(void* address, uint64_t count)
{
    lock_acquire(&lock);
    for (uint64_t i = 0; i < count; i++)
    {
        void* a = (void*)((uint64_t)address + i * PAGE_SIZE);
        bitmap[QWORD_INDEX(a)] &= ~(1ULL << BIT_INDEX(a));
    }    
    lock_release(&lock);
}

uint64_t pmm_total_amount(void)
{
    return pageAmount;
}

uint64_t pmm_free_amount(void)
{
    return pageAmount - pmm_reserved_amount();
}

uint64_t pmm_reserved_amount(void)
{
    uint64_t amount = 0;
    for (uint64_t i = 0; i < pageAmount; ++i) 
    {
        if (pmm_is_reserved((void*)(i * PAGE_SIZE)))
        {
            amount++;
        }
    }
    return amount;
}

uint64_t pmm_usable_amount(void)
{
    return usablePageAmount;
}

uint64_t pmm_unusable_amount(void)
{
    return pageAmount - usablePageAmount;
}