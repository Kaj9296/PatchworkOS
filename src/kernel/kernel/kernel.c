#include "kernel.h"

#include "gdt/gdt.h"
#include "tty/tty.h"
#include "idt/idt.h"
#include "heap/heap.h"
#include "utils/utils.h"
#include "ram_disk/ram_disk.h"
#include "page_allocator/page_allocator.h"
#include "io/io.h"
#include "hpet/hpet.h"
#include "time/time.h"
#include "tss/tss.h"
#include "apic/apic.h"
#include "global_heap/global_heap.h"
#include "madt/madt.h"

#include "master/master.h"
#include "worker_pool/worker_pool.h"

#include "../common.h"

void kernel_init(BootInfo* bootInfo)
{    
    tty_init(bootInfo->framebuffer, bootInfo->font);
    tty_print("Hello from the kernel!\n\r");

    page_allocator_init(bootInfo->memoryMap);
    page_directory_init(bootInfo->memoryMap, bootInfo->framebuffer);
    heap_init();
    global_heap_init();

    tss_init();
    gdt_init();

    rsdt_init(bootInfo->xsdp);
    madt_init();
    apic_init();

    ram_disk_init(bootInfo->rootDirectory);
    
    hpet_init();
    
    master_init();
    worker_pool_init();
}