#pragma once

#include "memory/memory.h"

#include <common/boot_info/boot_info.h>

//TODO: Redo this, add better flag handling, add global pages etc.

#define USER_ADDRESS_SPACE_TOP 0x100000000
#define USER_ADDRESS_SPACE_BOTTOM 0

#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_READ_WRITE (1 << 1)
#define PAGE_FLAG_USER_SUPERVISOR (1 << 2)
#define PAGE_FLAG_WRITE_TROUGH (1 << 3)
#define PAGE_FLAG_CACHE_DISABLED (1 << 4)
#define PAGE_FLAG_ACCESSED (1 << 5)
#define PAGE_FLAG_PAGE_SIZE (1 << 7)

#define PAGE_DIRECTORY_GET_FLAG(entry, flag) (((entry) >> (flag)) & 1)
#define PAGE_DIRECTORY_GET_ADDRESS(entry) ((void*)((entry) & 0x000FFFFFFFFFF000))

#define PAGE_DIRECTORY_GET_INDEX(address, level) (((uint64_t)address & ((uint64_t)0x1FF << ((level - 1) * 9 + 12))) >> ((level - 1) * 9 + 12))

#define PAGE_DIRECTORY_ENTRY_CREATE(address, flags) (((((uint64_t)(address) >> 12) & 0x000000FFFFFFFFFF) << 12) | ((uint64_t)flags | (uint64_t)PAGE_FLAG_PRESENT))

#define PAGE_DIRECTORY_LOAD(pageDirectory) asm volatile ("movq %0, %%cr3" : : "r" ((uint64_t)pageDirectory))

typedef uint64_t PageDirectoryEntry;

typedef struct
{ 
    PageDirectoryEntry entries[512];
} PageDirectory;

extern void page_directory_invalidate_page(void* virtualAddress);

PageDirectory* page_directory_new();

void page_directory_map_pages(PageDirectory* pageDirectory, void* virtualAddress, void* physicalAddress, uint64_t pageAmount, uint16_t flags);

void page_directory_map(PageDirectory* pageDirectory, void* virtualAddress, void* physicalAddress, uint16_t flags);

void* page_directory_get_physical_address(PageDirectory const* pageDirectory, void* virtualAddress);

void page_directory_free(PageDirectory* pageDirectory);