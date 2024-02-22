#pragma once

#include <stdint.h>

#define EFI_GET_DESCRIPTOR(memoryMap, index) (EfiMemoryDescriptor*)((uint64_t)(memoryMap)->base + ((index) * (memoryMap)->descriptorSize))

typedef enum
{
	EFI_RESERVED,
	EFI_LOADER_CODE,
	EFI_LOADER_DATA,
	EFI_BOOT_SERVICES_CODE,
	EFI_BOOT_SERVICES_DATA,
	EFI_RUNTIME_SERVICES_CODE,
	EFI_RUNTIME_SERVICES_DATA,
	EFI_CONVENTIONAL_MEMORY,
	EFI_UNUSABLE_MEMORY,
	EFI_ACPI_RECLAIM_MEMORY,
	EFI_ACPI_MEMORY_NVS,
	EFI_MEMORY_MAPPED_IO,
	EFI_MEMORY_MAPPED_IO_PORT_SPACE,
	EFI_PAL_CODE,
	EFI_PERSISTENT_MEMORY,
	EFI_MAX_MEMORY_TYPE
} EFI_MEMORY_TYPE;

uint8_t is_memory_type_usable(uint64_t memoryType);

uint8_t is_memory_type_reserved(uint64_t memoryType);