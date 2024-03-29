#pragma once

#include <stddef.h>
#include <stdint.h>

#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE (1 << 1)
#define PAGE_FLAG_USER_SUPERVISOR (1 << 2)
#define PAGE_FLAG_WRITE_TROUGH (1 << 3)
#define PAGE_FLAG_CACHE_DISABLED (1 << 4)
#define PAGE_FLAG_ACCESSED (1 << 5)
#define PAGE_FLAG_GLOBAL (1 << 6)
#define PAGE_FLAG_PAGE_SIZE (1 << 7)

#define PAGE_ENTRY_AMOUNT 512

#define PAGE_ENTRY_GET_FLAG(entry, flag) \
    (((entry) & (flag)) != 0)

#define PAGE_ENTRY_GET_ADDRESS(entry) \
    ((void*)((entry) & 0x000FFFFFFFFFF000))

#define PAGE_TABLE_GET_INDEX(address, level) \
    (((uint64_t)(address) & ((uint64_t)0x1FF << (((level) - 1) * 9 + 12))) >> (((level) - 1) * 9 + 12))
    
#define PAGE_TABLE_INVALIDATE_PAGE(address) \
    asm volatile("invlpg %0" : : "m"(address))

typedef uint64_t PageEntry;

typedef struct
{ 
    PageEntry entries[PAGE_ENTRY_AMOUNT];
} PageTable;

PageTable* page_table_new(void);

void page_table_free(PageTable* table);

void page_table_load(PageTable* table);

void page_table_map_pages(PageTable* table, void* virtualAddress, void* physicalAddress, uint64_t pageAmount, uint16_t flags);

void page_table_map(PageTable* table, void* virtualAddress, void* physicalAddress, uint16_t flags);

void* page_table_physical_address(PageTable* table, void* virtualAddress);

void page_table_change_flags(PageTable* table, void* virtualAddress, uint16_t flags);
