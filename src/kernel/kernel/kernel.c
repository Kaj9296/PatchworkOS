#include "kernel.h"

#include "gdt/gdt.h"
#include "tty/tty.h"
#include "idt/idt.h"
#include "heap/heap.h"
#include "pmm/pmm.h"
#include "hpet/hpet.h"
#include "time/time.h"
#include "tss/tss.h"
#include "apic/apic.h"
#include "madt/madt.h"
#include "ram_disk/ram_disk.h"
#include "device_disk/device_disk.h"
#include "vfs/vfs.h"
#include "vmm/vmm.h"

#include "master/master.h"
#include "worker_pool/worker_pool.h"

#include <libc/string.h>
#include <common/common.h>

static void deallocate_boot_info(BootInfo* bootInfo)
{   
    tty_start_message("Deallocating boot info");

    EfiMemoryMap* memoryMap = &bootInfo->memoryMap;
    for (uint64_t i = 0; i < memoryMap->descriptorAmount; i++)
    {
        const EfiMemoryDescriptor* descriptor = vmm_physical_to_virtual(EFI_GET_DESCRIPTOR(memoryMap, i));

        if (descriptor->type == EFI_MEMORY_TYPE_BOOT_INFO)
        {
            pmm_unlock_pages(descriptor->physicalStart, descriptor->amountOfPages);
        }
    }

    tty_end_message(TTY_MESSAGE_OK);
}

void kernel_init(BootInfo* bootInfo)
{   
    asm volatile("cli");

    pmm_init(&bootInfo->memoryMap);
    vmm_init(&bootInfo->memoryMap);

    pmm_move_to_higher_half();
    bootInfo = vmm_physical_to_virtual(bootInfo);

    tty_init(&bootInfo->gopBuffer, &bootInfo->font);    
    tty_print("Hello from the kernel!\n");

    gdt_init();

    heap_init();
    
    rsdt_init(bootInfo->rsdp);
    hpet_init();
    madt_init();
    apic_init();

    time_init();

    pid_init();

    deallocate_boot_info(bootInfo);
    
    tty_print("USABLE LOCKED: ");
    tty_printi(((pmm_locked_amount() - pmm_unusable_amount()) * PAGE_SIZE) / 1048576);
    tty_print(" MB\n");

    tty_print("LOCKED: ");
    tty_printi(((pmm_locked_amount()) * PAGE_SIZE) / 1048576);
    tty_print(" MB\n");

    tty_print("UNLOCKED: ");
    tty_printi((pmm_unlocked_amount() * PAGE_SIZE) / 1048576);
    tty_print(" MB\n");

    uint64_t time = time_milliseconds();

    tty_print("\nStarting...\n");

    for (uint64_t i = 0; i < 100000; i++)
    {   
        void* address = vmm_allocate(1, PAGE_FLAG_WRITE);
    }

    tty_print("\nTime: ");
    tty_printi(time_milliseconds() - time);
    tty_print(" MS\n");

    while (1)
    {
        asm volatile("hlt");
    }

    master_init();
    worker_pool_init();
    
    vfs_init();
    device_disk_init();
    ram_disk_init(bootInfo->ramRoot);
}