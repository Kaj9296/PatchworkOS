#pragma once

#include <stdint.h>

#define IDT_INTERRUPT 0xEE //Disables interrupts
#define IDT_TRAP 0xEF //Does not disable interrupts

typedef struct __attribute__((packed))
{
	uint16_t isrLow;      
	uint16_t codeSegment;
	uint8_t	ist;
	uint8_t attributes;
	uint16_t isrMid;
	uint32_t isrHigh;
	uint32_t reserved;
} IdtEntry;

typedef struct __attribute__((packed))
{
	uint16_t size;
	uint64_t offset;
} IdtDesc;

extern IdtEntry idt[];

void idt_init();

void remap_pic();

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);

void enable_interrupts();

void disable_interrupts();
