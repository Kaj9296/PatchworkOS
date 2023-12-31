#pragma once

#include <stdint.h>

#include "interrupt_frame/interrupt_frame.h"
#include "page_directory/page_directory.h"

#define IRQ_BASE 0x20
#define IRQ_TIMER 0x0
#define IRQ_KEYBOARD 0x1
#define IRQ_CASCADE 0x2
#define IRQ_COM2 0x3
#define IRQ_COM1 0x4
#define IRQ_LPT2 0x5
#define IRQ_FLOPPY 0x6
#define IRQ_LPT1 0x7
#define IRQ_CMOS 0x8
#define IRQ_FREE1 0x9
#define IRQ_FREE2 0xA
#define IRQ_FREE3 0xB
#define IRQ_PS2_MOUSE 0xC
#define IRQ_FPU 0xD
#define IRQ_PRIMARY_ATA_HARD_DISK 0xE
#define IRQ_SECONDARY_ATA_HARD_DISK 0xF

#define IRQ_AMOUNT 0x10

void interrupts_init();

void interrupts_enable();

void interrupts_disable();

void interrupt_vectors_map(PageDirectory* pageDirectory);

void interrupt_handler(InterruptFrame* interruptFrame);

void irq_handler(InterruptFrame* interruptFrame);

void ipi_handler(InterruptFrame* interruptFrame);

void exception_handler(InterruptFrame* interruptFrame);