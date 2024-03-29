#pragma once

#include <stdint.h>

#include <efi.h>
#include <efilib.h>

#include <common/boot_info/boot_info.h>

void* memory_allocate_pages(uint64_t pageAmount, uint64_t memoryType);

void* memory_allocate_pool(uint64_t size, uint64_t memoryType);

void memory_free_pool(void* pool);

void memory_map_populate(EfiMemoryMap* memoryMap);
